# from scipy.io import loadmat
from mat73 import loadmat
import numpy as np
from sys import argv


if __name__ == "__main__":
    # Convert input file name from .mat to .txt with header and index column
    mat_data = loadmat(argv[1])['fea']
    indexes = np.arange(mat_data.shape[0], dtype=np.uint8).reshape(-1, 1)
    np.savetxt(argv[1].replace('.mat', '.txt'), np.column_stack([indexes, mat_data]), delimiter=' ', newline='\n')

    # TODO: Still needs header line! I did this manually for our set for now.