# ----------------------------------------------------------------------
#
# File: test.py
#
# Last edited: 21.04.2024
# Copyright (C) 2024, ETH Zurich and University of Bologna.
#
# Author: Cristian Cioflan, ETH Zurich
#         Viviane Potocnik, ETH Zurich
#         Moritz Scherer, ETH Zurich
#         Victor Jung, ETH Zurich
#
# ----------------------------------------------------------------------
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the License); you may
# not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an AS IS BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import torch
import argparse
import json
import os
import glob

import numpy as np

from dataclasses import dataclass, field
from typing import Union, Optional
from rich.progress import track
from torch import nn, fx

import quantlib.algorithms as qa
from torch.utils.data import DataLoader
from dataset import DatasetProcessor
from datagenerator import DatasetCreator
from utils import parameter_generation
from dscnn import DSCNN

# import the DORY backend
from quantlib.backends.dory import export_net, DORYHarmonizePass
# import the PACT/TQT integerization pass
from quantlib.editing.fx.passes.pact import IntegerizePACTNetPass
from quantlib.editing.fx.util import module_of_node
from quantlib.algorithms.pact.pact_ops import *
# organize quantization functions, datasets and transforms by network
from pactnet import pact_recipe as quantize_net, get_pact_controllers as controllers_net

from quantUtils import roundTensors

import soundfile as sf

# import tensorflow as tf

# TODO: Functional dataset management
mdataset = None
mdataloader = None

@dataclass
class QuantUtil:
    problem : str
    topo : str
    quantize : callable
    get_controllers : callable
    network : type
    in_shape : tuple
    eps_in : float
    D : int
    bs : int
    get_in_shape : callable
    load_dataset_fn : callable
    transform : type
    n_levels_in : int
    export_fn : callable
    code_size : int
    network_args : dict = field(default_factory=dict)
    quant_transform_args : dict = field(default_factory=dict)


# _MNIST_EPS = 0.99
_MNIST_EPS = 0.39 # for 0-255 data
# _MNIST_EPS = 0.0328 # for standardized 0-1 data 

# batch size is per device, determined on Nvidia RTX2080. You may have to change
# this if you have different GPUs
_QUANT_UTILS = {
    'DSCNN':  QuantUtil(problem='MNIST', topo='DSCNN', quantize=quantize_net, get_controllers=controllers_net, network=DSCNN, in_shape=(1,1,49,10), eps_in=_MNIST_EPS, D=2**19, bs=256, get_in_shape=None, load_dataset_fn=DatasetProcessor.get_dataset, transform=None, quant_transform_args={'n_q':256}, n_levels_in=256, export_fn=export_net, code_size=150000)
}

def get_network(key : str, exp_id : int, ckpt_id : Union[int, str], quantized=False, pretrained='model.pth'):
    with open('config_net_tqt_8b.json', 'r') as fp:
        cfg = json.load(fp)
    qu = _QUANT_UTILS[key]
    quant_cfg = cfg['network']['quantize']['kwargs']
    ctrl_cfg = cfg['training']['quantize']['kwargs']
    net_cfg = cfg['network']['kwargs']
    if qu.in_shape is None:
        qu.in_shape = qu.get_in_shape(cfg)
        _QUANT_UTILS[key].in_shape = qu.in_shape

    net_cfg.update(qu.network_args)
    # net = qu.network(**net_cfg)
    net = qu.network()

    # print ("Network instantiated.")
    # print (net)

    # Load pretrained network
    net.load_state_dict(torch.load(pretrained, map_location='cpu'))

    print("Validation of FP32 loaded network")
    validate(net, mdataloader, 10, n_valid_batches=10)

    if not quantized:
        print ("The network is not to be quantized. Returning...")
        return net.eval()
    quant_net = qu.quantize(net, **quant_cfg)

    # we don't want to train this network anymore
    return quant_net


def validate(net : nn.Module, dl : torch.utils.data.DataLoader, print_interval : int = 10, n_valid_batches : int = None, integerized : bool = False, eps: float = -1):
    net = net.eval()
    # we assume that the net is on CPU as this is required for some
    # integerization passes
    device = 'cpu'

    n_tot = 0
    n_correct = 0

    for i, batched_input in enumerate(dl):
        xb, yb = batched_input

        if integerized:
            xb = roundTensors([xb], torch.tensor((eps,)))[0]
            xb = xb/torch.tensor((eps,))
            xb = xb.to(torch.int).to(torch.float32) # sufficient if eps==1
            
            # import IPython; IPython.embed()

        yn = net(xb.to(device))

        n_tot += xb.shape[0]

        n_correct += (yn.to('cpu').argmax(dim=1) == yb).sum()
        if ((i+1)%print_interval == 0):
            print(f'Accuracy after {i+1} batches: {n_correct/n_tot}')
        if (i+1) == n_valid_batches:
            break

    print(f'Final accuracy: {n_correct/n_tot}')
    net.to('cpu')


def main():
    
    parser = argparse.ArgumentParser()
    parser.add_argument("--net", type=str, default='DSCNN', help='Network to quantize')
    parser.add_argument("--pretrained", type=str, default='model_nlkws.pth', help='Path to pretrained model {model_nlkws,model_nakws}.pth.')
    parser.add_argument('--fix_channels', action='store_true', help='Fix channels of conv layers for compatibility with DORY')
    parser.add_argument('--no_dory_harmonize', action='store_true',
                        help='If supplied, don\'t align averagePool nodes\' associated requantization nodes and replace adders with DORYAdders')
    parser.add_argument('--word_align_channels', action='store_true',
                        help='Fix channels of conv layers so (#input_ch * #input_bits) is a multiple of 32 to work around XpulpNN HW bug')
    parser.add_argument('--requant_node', action='store_true',
                        help='Export RequantShift nodes instead of mul-add-div sequences in ONNX graph')
    parser.add_argument('--clip_inputs', action='store_true',
                        help='ghettofix to clip inputs to be unsigned')
    parser.add_argument('--config_net_file', type=str, default='config_net_tqt_8b.json', help = 'Network configuration file')
    parser.add_argument('--config_env_file', type=str, default='config_docker_env.json', help = 'Environment configuration file')
    parser.add_argument('--input', type=str, default=None, help = 'WAV file to predict')

    args = vars(parser.parse_args())

    # Parameter generation
    environment_parameters, preprocessing_parameters, training_parameters, experimental_parameters = parameter_generation(args) 

    # Device setup
    os.environ["CUDA_VISIBLE_DEVICES"] = environment_parameters['device_id']
    if torch.cuda.is_available() and environment_parameters['device'] == 'gpu':
        device = torch.device('cuda')        
    else:
        device = torch.device('cpu')
    device = torch.device('cpu')
    print (torch.version.__version__)
    print (device)

    torch.manual_seed(0)
    np.random.seed(0)


    audio_processor = DatasetCreator(environment_parameters, training_parameters, preprocessing_parameters, experimental_parameters)
    # TODO: Functional dataset management
    # print ("Created audio_processor")
    global mdataset
    # print ("Imported mdataset")
    mdataset = DatasetProcessor("training", audio_processor, training_parameters, task = -1, device = 'cpu')
    # print ("Created mdataset")
    global mdataloader
    # print ("Imported mdataloader")
    mdataloader = DataLoader(mdataset, batch_size=training_parameters['batch_size'], shuffle=False, num_workers=0)
    # print ("Created mdataloader")


    # Loading pre-trained network 
    pretrained = args['pretrained']
    qnet = get_network(key = args['net'], exp_id=0, ckpt_id=0, quantized=False, pretrained = pretrained)

    files = []
    if (args['input'] is None):
        path = './.'
        for filename in glob.glob(os.path.join(path, '*.wav')):
            files.append(filename) 
    else:
        files.append(args['input'])

    for file in files:

        # Read WAV    
        sf_loader, _ = sf.read(file)
        wav_file = torch.from_numpy(sf_loader).float()

        desired_samples = 16000

        length_minus_window = desired_samples - 640
        if (length_minus_window < 0):
            spectrogram_length = 0
        else:
            spectrogram_length = 1 + int(length_minus_window / 320)

        time_shift_amount = np.random.randint(-3200, 3200)
        if time_shift_amount > 0:
            time_shift_padding = [[time_shift_amount, 0], [0, 0]]
            time_shift_offset = [0, 0]
        else:
            time_shift_padding = [[0, -time_shift_amount], [0, 0]]
            time_shift_offset = [-time_shift_amount, 0]

        # Padding wrt the time shift offset
        pad_tuple=tuple(time_shift_padding[0])
        padded_foreground = torch.nn.ConstantPad1d(pad_tuple,0)(wav_file)
        wav_file = padded_foreground[time_shift_offset[0]:time_shift_offset[0]+desired_samples]
        
        import torchaudio
     
        if (device == 'cuda'):
            torch.set_default_tensor_type('torch.cuda.FloatTensor')
 
        melkwargs={ 'n_fft':1024, 'win_length':640, 'hop_length':320,
                             'f_min':20, 'f_max':4000, 'n_mels':10}
        mfcc_transformation = torchaudio.transforms.MFCC(n_mfcc=10, sample_rate=desired_samples, melkwargs=melkwargs, log_mels=True, norm='ortho')
        data = mfcc_transformation(wav_file)
        

        data = torch.transpose(data[:,:spectrogram_length], 0, 1)
        
        # Add batch & channel dimension
        data = data[None, None, :, :]

        # Match DORY
        data = torch.clamp(data + 128, 0, 255)

        flatdata = data.flatten()
        for i in range (0, 490):
            print ("feat_char[", i , "] = ", int(flatdata[i].numpy()))


        if (device == 'cuda'):
            torch.set_default_tensor_type('torch.FloatTensor')    

        output = qnet(data)
        print ("Prediction: ", output)
        # uknown, silence, yes, no, up, down, left, right, on, off, stop, go
        if (torch.argmax(output) == 0):
            print ("Unknown")
        elif  (torch.argmax(output) == 1):
            print ("Silence")
        elif  (torch.argmax(output) == 2):
            print ("Yes")
        elif  (torch.argmax(output) == 3):
            print ("No")
        elif  (torch.argmax(output) == 4):
            print ("Up")
        elif  (torch.argmax(output) == 5):
            print ("Down")
        elif  (torch.argmax(output) == 6):
            print ("Left")
        elif  (torch.argmax(output) == 7):
            print ("Right")
        elif  (torch.argmax(output) == 8):
            print ("On")
        elif  (torch.argmax(output) == 9):
            print ("Off")
        elif  (torch.argmax(output) == 10):
            print ("Stop")
        elif  (torch.argmax(output) == 11):
            print ("Go")



if __name__ == "__main__":
    main( )
