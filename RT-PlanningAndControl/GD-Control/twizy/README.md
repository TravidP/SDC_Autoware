# Overview

## Launch order
The main launch file is ```twizy_control.launch.xml``` which includes ```rt_tier4_control_component.launch.xml``` which in turn includes ```gd_control.launch.xml```.
```
`twizy_control.launch.xml`
            |
            |
            |
            |
            V
`rt_tier4_control_component.launch.xml`
            |
            |
            |
            |
            V
`gd_control.launch.xml`
```

The approach is not the cleanest but we adopted it like so for ease when updating autoware, since after an update it becomes much easier to check for differences in the configuration files as well as the launch files.
Also not all the configurations for the packages are present in the top-level launch file of autoware's control module so we had to use/copy the nested launch files as well.

## TODO
- Decide on a structure/approach to handle this in a more concise manner.