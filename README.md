A tool using `gmx_rescue` to salvage frames from a corrupted gromacs
trajectory.

## Errors:

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
## Solutions:

### Magic Number Error

#### First Part
Assuming that we have a corrupted trajectory, `corrupted.xtc`, we
first run `gmx check -f corrupted.xtc`. The magic number error will be
confirmed with an output similar to:

```
Command line:
gmx check -f corrupted.xtc

Checking file corrupted.xtc
Reading frame       0 time 89142.000   
# Atoms  135145
Precision 0.001 (nm)
Reading frame      16 time 89158.000   Warning at frame 16: there are 4 particles with all coordinates zero

-------------------------------------------------------
Program gmx check, VERSION 5.1.3

Fatal error:
Magic Number Error in XTC file (read -1471867024, should be 1995)
For more information and tips for troubleshooting, please check the GROMACS
website at http://www.gromacs.org/Documentation/Errors
-------------------------------------------------------
```

We see from the output that one of the last frame read was frame 16. In the
very next statement the output tells us explicitly that frame 16 had
an error. Note that the output sometimes does not tell us which frame
is directly responsible. We then check if up to frame 16 is
fine. Using `gmx trjconv` with the time of frame 16 (89158.000 ps in this
case):

`gmx trjconv -f corrupted.xtc -e 89158 -o part1.xtc`

If this gives a failure, then you know your `part1.xtc` contains part
of the corruption. Simply run `gmx trjconv` again one frame
shorter. In this case our time-step is 1 ps and the next frame to check
is frame 15.

`gmx trjconv -f corrupted.xtc -e 89157 -o part1.xtc`

Continue this process until you do not get an error. On the other hand,
if the command works the first invocation of `gmx trjconv` then you
might want to keep increasing the number of frames until you get an
error and then take the last good one. This could save you a few
frames. You will have to move forward or backward in frame number
depending on how long the trajectory is since `gmx check` begins to
refresh the screen on a much longer interval when there are a lot of
frames.

#### Second Part

Once you have `part1.xtc` the next step is to find out how many frames
are corrupted past where you cut `part1.xtc` off of the original
trajectory. This is where you `source` the `first_aid.sh`
file. Make sure that `$GMXRESCUE` points to the `gmx_rescue64`
binary and then `source first_aid.sh`. This defines functions that
can then be used in your shell.

We can then use new function `find_next_good_frame` to recursively move
through the trajectory until either a good frame is found or we run
out of frames, in which case you are going to just have to use
`part1.xtc` as your fixed trajectory.  Let's say that we know the last
frame was frame 15, which was included in `part1.xtc`.

`find_next_good_frame corrupted.xtc 16 part2.xtc`

The function will then create `part2.xtc` if it can find a good
frame. Once we have `part1.xtc` and `part2.xtc` we simply run
`combine_and_check part1.xtc part2.xtc newtrajectory.xtc`. This will
concatenate the two trajectories into `newtrajectory.xtc` and run a
final `gmx check` on it for good measure. If everything went well then
`new_trajectory.xtc` should be the result you were after.

### Floating point exception

Assuming that we have a corrupted trajectory, `corrupted.xtc`, we
first run `gmx check -f corrupted.xtc`. The floating point exception will be
confirmed with an output similar to:

```
Command line:
gmx check -f cor.md.part0140.xtc

Checking file cor.md.part0140.xtc
Reading frame       0 time 88437.000   
# Atoms  132502
Precision 0.001 (nm)
Reading frame      15 time 88452.000   Floating point exception (core dumped)
```

This is where you `source` the `first_aid.sh`
file. Make sure that `$GMXRESCUE` points to the `gmx_rescue64`
binary and then `source first_aid.sh`. This defines functions that
can then be used in your shell. Use the command:

`strip_front_section corrupted.xtc part1.xtc`

This will quickly find the last good frame and salvage all of the
frames up until that point, storing them in `part1.xtc`. From here you
can continue on with the second part of the Magic Number Error above, as they are identical.