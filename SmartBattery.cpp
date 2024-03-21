/*

*/
#include "SmartBattery.h"


Battery_Mode::Battery_Mode(unsigned int BitMap) {
  BM = BitMap;
}

Battery_Mode::Battery_Mode(bool ICC, bool PBS, bool CF, bool CCE, bool PB, bool AM, bool ChM, bool CM) {
  BM |= ICC << 0x0000;
  BM |= PBS << 0x0001;
  BM |= CF << 0x0006;
  BM |= CCE << 0x0007;
  BM |= PB << 0x0008;
  BM |= AM << 12;
  BM |= ChM << 13;
  BM |= CM << 14;

  // setting CM should prompt to reread the Battery instance
}

bool Battery_Mode::InternalChargeController() const {
  return (BM & 0x0001 != 0);
}
bool Battery_Mode::PrimaryBatterySupport() const {
  return (BM & 0x0002 != 0);
}

bool Battery_Mode::ConditionFlag() const {
  return (BM & 0x0004 != 0);
}

bool Battery_Mode::ChargeControllerEnabled() const {
  return (BM & 0x0008 != 0);
}

bool Battery_Mode::PrimaryBattery() const {
  return (BM & 0x0010 != 0);
}

bool Battery_Mode::AlarmMode() const {
  return (BM & 0x2000 != 0);
}

bool Battery_Mode::ChargerMode() const {
  return (BM & 0x4000 != 0);
}

bool Battery_Mode::CapacityMode() const {
  return (BM & 0x8000 != 0);
}
word Battery_Mode::asWord() const {
  return BM;
}

Battery_Status::Battery_Status(uint16_t BitMap) {
  BS = BitMap;
}

Battery_Status::Battery_Status() {}

bool Battery_Status::has_alarm(Alarm Alarm) const {
  return ((BS & 0xff00) & Alarm);
}
bool Battery_Status::has_alarm() {
  bool alrm = false;
  return (highByte(BS));
}

bool Battery_Status::is(Status Status) const {
  return ((BS & 0x00f0) & Status);
}
bool Battery_Status::has_error(Error Error) const {
  return ((BS & 0x000f) & Error);
}

String* Battery_Status::asStringArr() const {
  String stats[16];
  byte j = 0;
  if ((BS & 0x000f) == 0) {
    stats[++j] = "ERR::OK";
  }
  if ((BS & 0x000f) == 1) {
    stats[++j] = "ERR::Busy";
  }
  if ((BS & 0x000f) == 2) {
    stats[++j] = "ERR::ReservedCommand";
  }
  if ((BS & 0x000f) == 3) {
    stats[++j] = "ERRUnsupportedCommand";
  }
  if ((BS & 0x000f) == 4) {
    stats[++j] = "ERR:AccessDenied";
  }
  if ((BS & 0x000f) == 5) {
    stats[++j] = "ERR::Overflow_Underflow";
  }
  if ((BS & 0x000f) == 6) {
    stats[++j] = "ERR::BadSize";
  }
  if ((BS & 0x000f) == 7) {
    stats[++j] = "ERR::UnknownError";
  }
  if ((BS & 0x000f) > 7) {
    stats[++j] = "ERR::" + (BS & 0x000f);
  }
  if (bitRead(BS, 4)) {
    stats[++j] = "STAT::FULLY_DISCHARGED";
  }
  if (bitRead(BS, 5)) {
    stats[++j] = "STAT::FULLY_CHARGED";
  }
  if (bitRead(BS, 6)) {
    stats[++j] = "STAT::DISCHARGING";
  }
  if (bitRead(BS, 7)) {
    stats[++j] = "STAT::INITIALIZED";
  }
  if (bitRead(BS, 8)) {
    stats[++j] = "ALRM::REMAINING_TIME";
  }
  if (bitRead(BS, 9)) {
    stats[++j] = "ALRM::REMAINING_CAPACITY";
  }
  if (bitRead(BS, 10)) {
    stats[++j] = "ALRM::Reserved";
  }
  if (bitRead(BS, 11)) {
    stats[++j] = "ALRM::TERMINATE_DISCHARGE";
  }
  if (bitRead(BS, 12)) {
    stats[++j] = "ALRM::OVER_TEMP";
  }
  if (bitRead(BS, 13)) {
    stats[++j] = "ALRM::Reserved";
  }
  if (bitRead(BS, 14)) {
    stats[++j] = "ALRM::TERMINATE_CHARGE";
  }
  if (bitRead(BS, 10)) {
    stats[++j] = "ALRM::OVER_CHARGED";
  }

  String ret[++j];
  ret[0] = String(j - 1);
  for (byte i = 1; i < j; i++) {
    ret[i] = stats[i];
  }
  return ret;
}

String Battery_Status::asString() const {
  String* StrArr = asStringArr();
  byte len = StrArr[0].toInt();
  String ret = "";
  for (byte i = 0; i < len; i++) {
    ret.concat(", ");
    ret.concat(StrArr[i]);
  }
  return ret;
}

String* Battery_Status::meaning(uint16_t BitMap) {
  return Battery_Status(BitMap).asStringArr();
}

byte Battery_Status::error() const {
  return (BS & 0x000f);
}
word Battery_Status::asWord() const {
  return BS;
}

bool Battery_Status::operator==(const Battery_Status& rhs) const {
  return asWord() == rhs.asWord();
}
bool Battery_Status::operator!=(const Battery_Status& rhs) const {
  return !(*this == rhs);
}

Specification_Info::Specification_Info(unsigned int BitMap) {
  SI = BitMap;
}

byte Specification_Info::Revision() const {
  return ((SI >> 0) & 0x8);
}

byte Specification_Info::Version() const {
  return ((SI >> 4) & 0x8);
}

byte Specification_Info::VScale() const {
  return ((SI >> 8) & 0x8);
}

byte Specification_Info::IPScale() const {
  return ((SI >> 12) & 0x8);
}

unsigned int Battery::designCapacity() const {
  return SMBus.fetchWord(address, 0x18);
}

unsigned int Battery::designVoltage() const {
  return SMBus.fetchWord(address, 0x19);
}

Specification_Info Battery::specificationInfo() const {
  return Specification_Info(SMBus.fetchWord(address, 0x1a));
}

unsigned int Battery::manufactureDate() const {
  return SMBus.fetchWord(address, 0x1b);
}

String Battery::manufactureDate(unsigned int date) const {
  byte day = (date >> 0) & 0x1F;
  byte month = (date >> 5) & 0xF;
  byte year = 1980 + ((date >> 9) & 0x3F);
  return String(year + '-' + month + '-' + day);
}

byte Battery::serialNumber() const {
  return SMBus.fetchWord(address, 0x1c);
}

String Battery::manufacturerName() const {
  byte dat[BUFFER_LENGTH];
  byte len;
  SMBus.readBlock(address, 0x20, dat, &len);
  return String((char*) dat);
}

String Battery::deviceName() const {
  byte dat[BUFFER_LENGTH];
  byte len;
  SMBus.readBlock(address, 0x21, dat, &len);
  return String((char*) dat);
}

String Battery::deviceChemistry() const {
  byte dat[BUFFER_LENGTH];
  byte len;
  SMBus.readBlock(address, 0x22, dat, &len);
  return String((char*) dat);
}

Battery::Battery(byte Address, void (*AlarmWarningHandler)(byte, byte)) {
  SMBus = SMBus;
  address = Address;
  awh = AlarmWarningHandler;
  SMBus.addReceiveHandler(SystemBus::iHandler(this, this));

  DesignCapacity = designCapacity();
  DesignVoltage = designVoltage();
  // SpecificationInfo = specificationInfo();
  ManufactureDate = manufactureDate();
  SerialNumber = serialNumber();
  ManufacturerName = manufacturerName();
  DeviceName = deviceName();
  DeviceChemistry = deviceChemistry();
}

Battery::Battery(void* instance) {
  self = static_cast<Battery*>(instance);
}

bool Battery::ReceiveHandler(char buf[], unsigned int length) {
  if (length == 2) {
    if ((byte) buf[0] == address) {
      // call awh with error bytes!
      awh(address, buf[1]);
      return true;
    }
  }
  return false;
}

Battery::Battery() {}

Battery_Mode Battery::BatteryMode() const {
  return Battery_Mode(SMBus.fetchWord(address, 0x03));
}

unsigned int Battery::Temperature() const {
  return SMBus.fetchWord(address, 0x08);
}

unsigned int Battery::Voltage() const {
  return SMBus.fetchWord(address, 0x09);
}

signed int Battery::Current() const {
  SMBus.fetchWord(address, 0x0a);
}

signed int Battery::CurrentAvg() const {
  SMBus.fetchWord(address, 0x0b);
}

unsigned int Battery::RelativeStateOfCharge() const {
  SMBus.fetchWord(address, 0x0d);
}

unsigned int Battery::AbsoluteStateOfCharge() const {
  return SMBus.fetchWord(address, 0x0e);
}

unsigned int Battery::RemainingCapacity() const {
  return SMBus.fetchWord(address, 0x0f);
}

unsigned int Battery::FullChargeCapacity() const {
  return SMBus.fetchWord(address, 0x10);
}

unsigned int Battery::RunTimeToEmpty() const {
  return SMBus.fetchWord(address, 0x11);
}

unsigned int Battery::RunTimeToEmptyAvg() const {
  return SMBus.fetchWord(address, 0x12);
}

unsigned int Battery::TimeToFullAvg() const {
  return SMBus.fetchWord(address, 0x13);
}

Battery_Status Battery::BatteryStatus() const {
  return Battery_Status(SMBus.fetchWord(address, 0x16));
}

unsigned int Battery::CycleCount() const {
  return SMBus.fetchWord(address, 0x17);
}

String Battery::ManufacturerData() const {
  byte dat[BUFFER_LENGTH];
  byte len;
  SMBus.readBlock(address, 0x23, dat, &len);
  return String((char*) dat);
}


