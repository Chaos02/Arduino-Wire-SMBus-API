/*

*/

#ifndef SmartBattery_h
#define SmartBattery_h

#include <Arduino.h>
#include "SMBus.h"

class Battery_Mode {
  unsigned int BM;
public:
  Battery_Mode(unsigned int BitMap);
  /// @brief reread your Battery after setting this!
  /// @param ICC InternalChargeController (READONLY!)
  /// @param PBS PrimaryBatterySupport  (READONLY!)
  /// @param CF ConditionFlag
  /// @param CCE ChargeControllerEnabled
  /// @param PB PrimaryBattery
  /// @param AM AlarmMode
  /// @param ChM ChargerMode
  /// @param CM CapacityMode
  Battery_Mode(bool ICC, bool PBS, bool CF, bool CCE, bool PB, bool AM, bool ChM, bool CM);

  bool InternalChargeController() const;
  bool PrimaryBatterySupport() const;
  /*
    false: battery OK
    true: Conditioning Cycle requested
    */
  bool ConditionFlag() const;
  /*
    false: Internal Charge controller disabled
    */
  bool ChargeControllerEnabled() const;
  /*
    false: battery operating in its secondary role
    */
  bool PrimaryBattery() const;
  /*
    false: enable AlarmWarning broadcasts to Host and Charger
    */
  bool AlarmMode() const;
  /*
    false: enable ChargingVoltage and Current broadcasts to charger
    */
  bool ChargerMode() const;
  /*
    false: report in mA or mAh
    true: report in 10mW or 10mWh
    */
  bool CapacityMode() const;
  word asWord() const;
};

class Battery_Status {
  uint16_t BS;
public:
  enum Error {
    OK = 0x0000,
    Busy = 0x0001,
    ReservedCommand = 0x0002,
    UnsupportedCommand = 0x0003,
    AccessDenied = 0x0004,
    Overflow_Underflow = 0x0005,
    BadSize = 0x0006,
    UnknownError = 0x0007
  };
  enum Status {
    FULLY_DISCHARGED = 0x0010,
    FULLY_CHARGED = 0x0020,
    DISCHARGING = 0x0040,
    INITIALIZED = 0x0080,
  };
  enum Alarm {
    RemainingTime = 0x0100,
    RemainingCapacity = 0x0200,
    RESERVED1 = 0x0400,
    TerminateDischarge = 0x0800,
    OverTemp = 0x1000,
    RESERVED2 = 0x2000,
    TerminateCharge = 0x4000,
    OverCharged = 0x8000
  };

  Battery_Status(uint16_t BitMap);

  /// @brief only for global variable init
  Battery_Status();

  bool has_alarm(Alarm Alarm) const;
  bool has_alarm();

  bool is(Status Status) const;
  bool has_error(Error Error) const;

  /// @brief retrieve all Status
  /// @return Array of all current active Status, Array[0] is length
  String* asStringArr() const;
  String asString() const;

  /// @brief retrieves the meaning of a bit in BatteryStatus
  /// @return String of what the value indicates
  static String* meaning(uint16_t BitMap);

  /*
    Returns stored error codes
    false: no error
    */
  byte error() const;
  word asWord() const;
  bool operator==(const Battery_Status& rhs) const;
  bool operator!=(const Battery_Status& rhs) const;

};

class Specification_Info {
  unsigned int SI;
public:
  Specification_Info(unsigned int BitMap);

  /*
    4 bit binary value 0001 - Version 1.0 and 1.1
    all other values reserved
    */
  byte Revision() const;

  /*
    4 bit binary value 0001 – Version 1.0
    0010 – Version 1.1
    0011 - Version 1.1 with optional PEC support
    all other values reserved
    */
  byte Version() const;

  /*
    4 bit binary value 0 - 3 (multiplies voltages* by 10 ^ VScale)
    *Note: Except ChargingVoltage() and ChargingCurrent() values.
    */
  byte VScale() const;

  /*
    4 bit binary value 0 - 3 (multiplies currents* and capacities by 10 ^ IPScale)
    *Note: Except ChargingVoltage() and ChargingCurrent() values.
    */
  byte IPScale() const;
};

/// @brief Implemented according to @link http://sbs-forum.org/specs/sbdat110.pdf
class Battery : protected SystemBus::iReceiveHandler {
  friend class Battery_Mode;
  friend class Battery_Status;
  friend class Specification_Info;

private:
  Battery* self;

  // Values that should NOT change during operation
  //SMBus SMBus;
  void (*awh)(byte, byte);

  /*
  */
  unsigned int designCapacity() const;

  /*
    */
  unsigned int designVoltage() const;

  /*
    */
  Specification_Info specificationInfo() const;

  unsigned int manufactureDate() const;

  /*
    Returns a YYYY-MM-DD String of the manufacture date
    */
  String manufactureDate(unsigned int date) const;

  byte serialNumber() const;

  String manufacturerName() const;

  String deviceName() const;

  String deviceChemistry() const;

public:
  byte address;
  unsigned int DesignCapacity;
  unsigned int DesignVoltage;
  // Specification_Info SpecificationInfo;
  String ManufactureDate;
  byte SerialNumber;
  String ManufacturerName;
  String DeviceName;
  String DeviceChemistry;

  /// @brief Set up a smart battery for polling
  /// @param SMBus an instance of the SMBus with which to fetch data
  /// @param Address the SMBus address of the battery device
  /// @param AlarmWarningHandler callback function pointer that handles an AlarmWarning event sent by the battery containing high byte of the Battery's Status
  Battery(byte Address, void (*AlarmWarningHandler)(byte, byte));

  Battery(void* instance);

  bool ReceiveHandler(char buf[], unsigned int length) override;

  Battery();

  Battery_Mode BatteryMode() const;

  /*
    Returns internal pack temperature in °cK (Kelvin / 10)
    */
  unsigned int Temperature() const;
  /*
    Returns battery terminal voltage in mV (Volt / 1000)
    */
  unsigned int Voltage() const;
  /*
    Returns current flowing through the terminals in mA (Ampere / 1000)
    positive for charge, negative for discharge
    */
  signed int Current() const;
  /*
    Returns one-minute rolling average of the current flowing through the terminals in mA (Ampere / 1000)
    positive for charge, negative for discharge
    */
  signed int CurrentAvg() const;

  /*
    Returns predicted remaining battery capacity expressed as a percentage of `FullChargeCapacity()`
    Range: 0% - 100%
    */
  unsigned int RelativeStateOfCharge() const;

  /*
    Returns predicted remaining battery capacity expressed as a percentage of DesignCapacity() (%)
    (Can return values greater than 100%)
    Range: 0% - 100%+
    */
  unsigned int AbsoluteStateOfCharge() const;

  /*
    Returns predicted remaining battery capacity in mAh or mWh
    (Capacity value is expressed in either current (mAh at a C/5 discharge rate) or power (10mWh at a P/5 discharge rate) depending on setting of BatteryMode()'s CAPACITY_MODE bit)
    */
  unsigned int RemainingCapacity() const;

  /*
    Returns predicted pack capacity in mAh or mWh
    (Capacity value is expressed in either current (mAh at a C/5 discharge rate) or power (10mWh at a P/5 discharge rate) depending on setting of BatteryMode()'s CAPACITY_MODE bit)
    */
  unsigned int FullChargeCapacity() const;

  unsigned int RunTimeToEmpty() const;

  unsigned int RunTimeToEmptyAvg() const;

  /*
    Returns one minute rolling average of predicted remaining time until Battery reaches full charge (minutes).
    65,535: not being charged
    */
  unsigned int TimeToFullAvg() const;

  Battery_Status BatteryStatus() const;

  /*
    */
  unsigned int CycleCount() const;

  String ManufacturerData() const;
};


#endif