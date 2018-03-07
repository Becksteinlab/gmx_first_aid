A tool using `gmx_rescue` to salvage frames from a corrupted gromacs
trajectory.

The two common corruptions I have found are:

    - Magic Number Errors

    ```
    Command line:
    gmx check -f cor.md.part0125.xtc

    Checking file cor.md.part0125.xtc
    Reading frame       0 time 89142.000   
    # Atoms  135145
    Precision 0.001 (nm)
    Reading frame      16 time 89158.000   Warning at frame 16: there are 4 particles with all coordinates zero

    -------------------------------------------------------
    Program gmx check, VERSION 5.1.3
    Source code file: /nfs/homes4/dldotson/Sysadmin/install/gromacs/source/gromacs-5.1.3/src/gromacs/fileio/xtcio.c, line: 90

    Fatal error:
    Magic Number Error in XTC file (read -1471867024, should be 1995)
    For more information and tips for troubleshooting, please check the GROMACS
    website at http://www.gromacs.org/Documentation/Errors
    -------------------------------------------------------
    ```

    - Floating point exception

    ```
    Command line:
    gmx check -f cor.md.part0140.xtc

    Checking file cor.md.part0140.xtc
    Reading frame       0 time 88437.000   
    # Atoms  132502
    Precision 0.001 (nm)
    Reading frame      15 time 88452.000   Floating point exception (core dumped)
    ```


