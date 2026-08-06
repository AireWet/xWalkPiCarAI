# xWalkBullFight

`xWalkBullFight` ports `example/10.bull_fight.py` through caller-owned PiCar-X
and computer-vision services. It selects red detection, tracks the target with
bounded camera angles, steers to the retained pan command, drives forward at
50 percent, and commands zero power while no target is visible.

The source's retained one-degree `dir_angle` calculation is preserved for
compatibility even though the upstream steering call uses `x_angle`. Physical
camera, servo, and motor execution remains opt-in.
