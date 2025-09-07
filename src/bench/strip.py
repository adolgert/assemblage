#!/usr/bin/env python3

import sys

def strip_assembly(filename):
    try:
        with open(filename, 'r') as f:
            for line in f:
                line = line.rstrip()
                
                # Skip blank lines
                if not line:
                    continue
                
                # Skip labels (lines ending with colon)
                if line.endswith(':'):
                    continue
                
                # Skip comments (lines starting with #)
                if line.lstrip().startswith('#'):
                    continue
                
                # Remove inline comments (everything after #)
                if '#' in line:
                    line = line.split('#', 1)[0].rstrip()
                    if not line:  # Skip if line becomes empty after removing comment
                        continue
                
                print(line)
                
    except FileNotFoundError:
        print(f"Error: File '{filename}' not found", file=sys.stderr)
        sys.exit(1)
    except IOError as e:
        print(f"Error reading file '{filename}': {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 strip.py <assembly_file>", file=sys.stderr)
        sys.exit(1)
    
    strip_assembly(sys.argv[1])