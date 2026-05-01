# Overview

## Launch order
The main launch file is ```citaro_control.launch.xml``` which includes ```rt_tier4_control_component.launch.xml``` which in turn includes ```rt_control.launch.xml```.
```
`citaro_control.launch.xml`
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
`rt_control.launch.xml`
```

See twizy's control README.md for more info.