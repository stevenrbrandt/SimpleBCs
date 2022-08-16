# SimpleBCs

To use the SimpleBCs "Simple Boundary Conditions" thorn, include the following in your thornlist:

```
!TARGET   = $ARR
!TYPE     = git
!URL      = https://github.com/stevenrbrandt/srbutils
!REPO_BRANCH = main
!REPO_PATH= $2
!CHECKOUT =
srbutils/SimpleBCs
```

After updating your repo, compiling, and building cactus, you can use this thorn by making a few simple additions to your parameter file. The following lines will apply periodic boundary conditions to grid functions "MyThorn:uu" and "MyThorn::vv".

```
# Activate Periodic Cactus
ActiveThorns = "PeriodicCarpet"

# Activate SimpleBCs
# By default RegisterAndApplySimpleBCs will run in MoL_PostStep only,
# but you can schedule it to run in other bins if appropriate.
ActiveThorns = "SimpleBCs"
SimpleBCS::bc_string = "none: MyThorn::uu MyThorn::vv"
```
