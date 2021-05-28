#!/usr/bin/env python3
import argparse

cli = argparse.ArgumentParser(description="Calculate average value from list of integers inside a file")
cli.add_argument("infile", type=str, action="store",
    help="Path where the measured data will be read from.")
args = cli.parse_args()

if __name__ == '__main__':
    try:
        with open(args.infile) as f:
            lines = f.read().splitlines()
    except FileNotFoundError:
        exit(1)
    data   = [int(line) for line in lines]
    print("%5f"%(sum(data)/float(len(data))))

