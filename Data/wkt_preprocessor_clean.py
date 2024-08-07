# Takes a file by name, where each line is "index_num \t geom_object \t metadata" and recreates it in the form "geom_object", dropping columns 0 and 2.

from sys import argv


if __name__ == "__main__":
    with open(argv[1], 'r') as f:
        lines = f.readlines() # TODO: change so that we only read and write a line (or so many lines) at a time, in case of files too large to fit in memory at once.

    with open(argv[1].split('.')[0] + ".wkt", 'a+') as o:
        for l in lines:
            parts = l.split('\t')
            if len(parts) > 1:
                o.write(parts[1] + '\n')