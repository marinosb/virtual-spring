Virtual Spring
==============

A virtual spring built for the Arduino (Arduino Due) using a Sanyo Denki industrial servomotor (QS1A05AA Driver & P06 motor). A GUI to control the spring is built in C# (WPF)

## Motivation

When performing real-world experimentation with springs, picking the correct spring type and spring constant (Hooke's Law) can be a strenuous procedure that involves physically switching springs. This project allows springs to be simulated using a servomotor, replacing the physical procesure of swapping springs with a click of a button.

## Implementation

This system will output the proper Torque on pin DAC0 of the Arduino Due, ranging from 0-3.3V. This signal should be shifted using OpAmps as appropriate, e.g. to (-3.3V, +3.3V) for our lab configuration

## Serial Protocol

* r: reset
* s&lt;int&gt;: stiffness
* v&lt;int&gt;: toruqe override
* p&lt;int&gt;: position offset


