from argparse import ArgumentParser
from typing import Literal
import numpy as np
from enum import Enum

class KeyType(Enum):
    email=0
    integer=1

class Distribution(Enum):
    uniform=0
    zip=1
    worst=2

parser = ArgumentParser()
parser.add_argument("--key_type",type=KeyType.__getitem__, default="integer")
parser.add_argument("--n_chars", type=int, default=12)
parser.add_argument("--n_keys", type=int, default=1_000_000)
parser.add_argument("--distribution", type=Distribution.__getitem__, default="uniform")
parser.add_argument("output", type=str)

args = parser.parse_args()
n_chars = args.n_chars
n_keys = args.n_keys
output = args.output
rng = np.random.default_rng()
dist = args.distribution
keys = rng.choice(10**n_chars, size=n_keys, replace=False)
keys = np.sort(keys)

with open(output, "w") as f:
    for key in keys:
        f.write(str(key).zfill(n_chars)+"\n")
