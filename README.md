# Assemblage

Examine code performance by probing the execution core and examining assembly.

This is run on a machine with `isolcpus=3,15` set. The cores 3 and 5 are SMT siblings.
If you want to undo that later, edit this file.
```bash
sudo nano /boot/efi/loader/entries/Pop_OS-current.conf
```

Run with
```bash
taskset -c 3 ./dependent
```
