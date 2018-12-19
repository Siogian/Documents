Distance To Empty
=================

- [Distance To Empty](#distance-to-empty)
  - [1. Description](#1-description)
  - [2. DTE definitions](#2-dte-definitions)
    - [2.1 Calibration](#21-calibration)
    - [2.2 Convention and definition](#22-convention-and-definition)
    - [2.3 Pseudocode data dictionary](#23-pseudocode-data-dictionary)
  - [3. Application Requirements](#3-application-requirements)
    - [3.1 DTE Calculation Strategy](#31-dte-calculation-strategy)
      - [3.1.1 Computing strategy](#311-computing-strategy)
      - [3.1.2 Depends](#312-depends)
    - [3.2 DTE Remaining Fuel Volume](#32-dte-remaining-fuel-volume)
      - [3.2.1 Computing strategy](#321-computing-strategy)
      - [3.2.2 Depends](#322-depends)
    - [3.3 DTE Average Fuel Consumption](#33-dte-average-fuel-consumption)
      - [3.3.1 Computing strategy](#331-computing-strategy)
      - [3.2.2 Depends](#322-depends-1)
  - [4. Exception Handling](#4-exception-handling)

## 1. Description

The distance to empty function is used to indicate to the driver the probable distance that can be driven until the remaining fuel volume in the tank is exhausted.

## 2. DTE definitions

### 2.1 Calibration

| Name                          | Description                                                              | Length | Unit    | Range | Default  |
| ----------------------------- | ------------------------------------------------------------------------ | ------ | ------- | ----- | -------- |
| **DTE_AFC_SAMPLE**            | The number of the AFC sample for DTE, once a kilometer                   | 1 Byte | /       | 0~255 | 30 * 1   |
| **DTE_OFFSET**                | The offset of DTE                                                        | 1 Byte | km      | 0~255 | 50 * 1   |
| **INITIAL_DTE_AFC**           | The initial AFC used in DTE formula                                      | 1 Byte | L/100km | 0~255 | 70 * 0.1 |
| **DTE_START_CALCULATE_SPEED** | DTE start to calculate when vehicle speed reach to the calibration value | 4 Bit  | km      | 0~15  | 3 * 0.5  |
| **DTE_UPDATE_MAX**            | The max update value of DTE                                              | 1 Byte | km      | 0~100 | 10 * 1   |

### 2.2 Convention and definition
- DTE_START_CALCULATE_TACHO
  
  DTE start to calculate when engine speed reach to the definition value, not a calibration value, default 300 rpm.

### 2.3 Pseudocode data dictionary

| Varialble & Method              | Descriptipn                                          |
| ------------------------------- | ---------------------------------------------------- |
| `DTE_FR`                        | The Remaining Fuel Volume since **Ft** reset         |
| `DTE_AFC`                       | The AFC of the latest **DTE_AFC_SAMPLE** km          |
| `DTETrip(n)`                    | The current trip meter since `DTE_AFC` Reset         |
| `DTEFC(n)`                      | total fuel consumption in n km since `DTE_AFC` Reset |
| `calculationEnableFlag_DTE`     | **DTE_ACTUAL**'s calculation enable flag, see 3.1.1  |
| `calculationEnableFlag_DTE_FR`  | **DTE_FR**'s calculation enable flag, see 3.2.1      |
| `calculationEnableFlag_DTE_AFC` | `DTE_AFC`'s calculation enable flag, see 3.3.1       |

calculationEnableFlag_DTE_FR
## 3. Application Requirements

### 3.1 DTE Calculation Strategy

#### 3.1.1 Computing strategy
$$DTE_{Actual} = Fr / DTE_{AFC} - DTE\_OFFSET \tag{Formula 1}$$

> Fr (`DTE_FR`) : Remaining Fuel Volume for calculating DTE  
> `DET_AFC` : the AFC which used to calculated DTE

```
WHEN (D5 -> D1) // KL30 OFF -> KL30 ON && KL15 ON
    Reset DTE_FR
    Reset DTE_AFC
    Reset DTE_Actual
    Refresh DET_DISPLAY
```
#### 3.1.2 Depends


### 3.2 DTE Remaining Fuel Volume

#### 3.2.1 Computing strategy

$$Fr = Ft - FC \tag{Formula 2}$$

> Ft : remaining fuel volume which after refuel or D5 -> D1  
> FC : current consumption so far from Ft reset, in order to calculate DTE

$$FC = \sum_{i=1}^n k * Fc_i \tag{Formula 3}$$
```
#define M 4

IF (Fr - Fs > 1)    // unit is L, The same below
    Set k = (Fr - Fs) / M + 1
ELSE IF (Fr - Fs < -1)
    Set k = 0.8
ELSE                // fabs(Fr - Fs) < 1
    Set k = 1
...
IF (FuelLevel_Status == ERROR) OR (FuelConsumption_Status == ERROR)
    Set calculationEnableFlag_DTE_FR = FALSE
ELSE
    Set calculationEnableFlag_DTE_FR = TRUE
```

> Fs : the remaining fuel volume of displayed on LCD with damp  
> Fr and FC should be calculated per 100 ms

#### 3.2.2 Depends

- FuelLevel
  - Ft using the Fuel Level when Refued or D5 -> D1
  - Calculate k using Fs (Display Fuel Level, unit L)
- FuelConsumption
  - Calculate FC using current Fuel Consumption (Cumulative value) 

### 3.3 DTE Average Fuel Consumption ###

The AFC of the latest **DTE_AFC_SAMPLE** km.

#### 3.3.1 Computing strategy

$$
    DTE_{AFC} = 
    \begin{cases}
       [DTEFC(n) + (DTE\_AFC\_SAMPLE – n) * INITIAL\_DTE\_AFC / 100] / DTE\_AFC\_SAMPLE & DTETrip(n) < DTE\_AFC\_SAMPLE \\
       DTEFC(DTE\_AFC\_SAMPLE) / DTE\_AFC\_SAMPLE & DTETrip(n) \geq DTE\_AFC\_SAMPLE
    \end{cases}
    \tag{Formula 4}
$$
```
IF (Speedometer_Status == ERROR) OR (Tachometer_Status == ERROR) OR (FuelConsumption_Status == ERROR) OR (Odometer_Status == ERROR)
    Set calculationEnableFlag_DTE_AFC = FALSE
ELSE IF (VehicleSpeed >= DTE_START_CALCULATE_SPEED) AND (EngineSpeed > DTE_START_CALCULATE_TACHO)
    Set calculationEnableFlag_DTE_AFC = TRUE
ELSE
    Set calculationEnableFlag_DTE_AFC = FALSE
```
#### 3.2.2 Depends

- Speedometer
  
  `DTE_AFC` calculation is enabled only when the vehicle speed is judged to be greater than or equel to **DTE_START_CALCULATE_SPEED**, to prevent sudden change of `DTE_AFC` caused by stopping fuel consumption or trailer.

- Tachometer
  
  `DTE_AFC` calculation is enabled only when the vehicle speed is judged to be greater than **DTE_START_CALCULATE_TACHO**, to prevent sudden change of `DTE_AFC` caused by stopping fuel consumption or trailer.

- FuelConsumption
  
  Calculate `DTEFC(n)`(Cumulative per 1 km) using current Fuel Consumption.

- Odometer

  Same as FuelConsumption.

## 4. Exception Handling

1. CAN::FuelRollingCounter 值异常 || 
   EMS 节点超时，
   Fc 停止计算并保持之前的值，DTE_AFC 停止计算并保持之前的值，DTE_Actual和DTE_Display保持不变；
2.  ```
    IF (FC_Status == ERROR || FuelLevel_Status == ERROR || Odometer_Status == ERROR || Speedometer == ERROR || Tachometer == ERROR)
        Set DTE_Status = ERROR
        Set DTE_Display = DET_DISPLAY_INVALID
    ```
    
   Ft 复位，Fc 复位，DTE_AFC基于失效前的值重新计算，DTE_Actual重新计算，DTE_Display立即更新；
3. D5->D1
   
   Ft 复位，Fc 复位，DTE_AFC复位，DTE_Actual重新计算，DTE_Display立即更新；
4. RefuelDetected || DTE Err Recover