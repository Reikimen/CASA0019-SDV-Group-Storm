---
typora-root-url: ./
---

# CASA0019-SDV-Group-Storm

![Gauge-AR-dashborad](/Image/Web-Showcase/Gauge-AR-dashborad.png)



## Wondering how much clothing you need to wear before walking out of the building?

Students at UCL EAST (including my group members and I) experience this situation quite often: because the classroom is heated or air-conditioned, a fairly comfortable office environment is maintained, resulting in a sudden cold wind that freezes you into a popsicle or a sun that is about to melt you when you go out. To solve this problem, our group designed a dial that displays the outdoor body temperature as a reminder to dress and undress.

This is the URL used for showcase: https://codepen.io/Reikimen/pen/NPKRRJR

## How can we define the criteria for dressing?

- **OutTemp_C**: Outside temperature (Celsius).
- **WindSpeed_kph**: Wind speed (kilometers per hour).
- **outHumidity**: Relative humidity (percentage).
- **waterVaporPressure**: Water vapor pressure.

1. **Calculate Water Vapor Pressure:**

```cpp
   waterVaporPressure = (outHumidity / 100) * 6.105 * exp((17.27 * OutTemp_C) / (237.7 + OutTemp_C));
```

- This formula calculates the water vapor pressure in hectopascals (hPa).

2. **Calculate Apparent Temperature:**

```cpp
   apparentTemperature = (1.04 * OutTemp_C) + (0.2 * waterVaporPressure) - (0.65 * WindSpeed_kph) - 2.7;
```

- This formula combines temperature, humidity, and wind speed to calculate the apparent temperature.

3. **Find the ups, comfortable and collars of the Apparent Temp:**

	- **High Temperature Condition:**

	```cpp
	 OutTemp_C = 38.1;
	 outHumidity = 100;
	 WindSpeed_kph = 0;
	 Apparent Temperature: 50.19
	```

	- **Low Temperature Condition:**

	```cpp
	 OutTemp_C = -9.2;
	 outHumidity = 0;
	 WindSpeed_kph = 60;
	 Apparent Temperature: -54.48
	```

	- **Comfortable Condition:**

	```cpp
	 OutTemp_C = 21;
	 outHumidity = 50;
	 WindSpeed_kph = 0.2;
	 Apparent Temperature: 21.49 -> 21.5
	```

2. **Calculate Apparent Temperature Index:**

```cpp
   Apparent_Index = (apparentTemperature + 11.5) * 1.5;
   if (Apparent_Index < 0) Apparent_Index = 0;
   if (Apparent_Index > 90) Apparent_Index = 90;
```

- Converts the apparent temperature to an index range (0-90).
- Adds 11.5 to the apparent temperature, then multiplies by 1.5.
- Ensures the index is between 0 and 90.

### **Summary (still processing):**

This code calculates the water vapor pressure and apparent temperature using temperature, humidity, and wind speed data. It then converts the apparent temperature to an index, which can be used to determine the appropriate amount of clothing to wear based on current weather conditions.



