#!/bin/bash
config-pin P8_19 pwm

cd /sys/class/pwm/pwmchip7
echo 0 > export
cd pwm-7:0
echo 1000000000 > period
echo 250000000 > duty_cycle
echo 1 > enable

cd /sys/class/gpio/gpio67
echo in > direction
echo rising > edge