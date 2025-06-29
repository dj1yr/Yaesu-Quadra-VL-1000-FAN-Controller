# Yaesu-Quadra-VL-1000-FAN-Controller

Hi,

I have had a Quadra with a late serial number for a long time.
It's a nice PA that I really enjoy using, but what has always bothered me a lot is the PA's fan noise, the fans are always running at full speed.
These are controlled by the µC of the PA, this switches the fans on, I have checked this chain, everything is in order and there is no defect (µC -> Q8004 etc.)
Why Yaesu has solved this in this way is a mystery to me.

I have started to design a fan controller that uses the original connections so that no changes need to be made to the original hardware.

The controller is based on an Atmega8, 2x I2C temperature sensors (DS18B20) and two PWM FET stages. (FAN A+B and FAN C)
One temperature sensor is attached to the RF block, one to the tuner.

Unfortunately, the original fans, which are actually very good, cannot be throttled, they then make a lot of noise. 
I have replaced my fans with Noctua NF-A12-25 in black and the FAN C with Noctua NF-A8 PWM chromax.black.swap,
these have a good static pressure with low noise and can be regulated very well (I like to use them in other PA projects).

The controller controls the fans, the lower limit temperature is 20°C, from this point the fans switch off, when PTT is on, both fans are controlled with 10% FAN C or 20% FAN A+B, when the temperature rises the fans turn faster, at approx. 40°C the fans run at maximum.
FAN C is PTT dependent, fans A+B run independently.
There is also a switching output that can be connected in parallel to “PA-ALM” (J1009/J1010), which would set the PA to overtemperature protect mode if limit values/heat sensors are exceeded.
A query of the fan speed has been prepared, but is not yet evaluated.

The QRL gives me the opportunity to take flow measurements, and I noticed that the two rear fans influence each other, that they can “see” each other, i.e. the air flow is partially canceled out and the efficiency drops by 20% in some cases.
I made two fan frames for my PA (3d printed at JLPCB), which reduced the problem to <2% influence.

So far everything is working fine, the PA doesn't get any warmer than before (checked with IR camera) and is now finally quiet when it is only in standby or used without a test.

The project was originally created in Eagle, then converted to KICAD, the software is written in the Arduino IDE, the fan frame in Inventor.


73´ René
