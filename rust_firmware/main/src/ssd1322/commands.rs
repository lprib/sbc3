pub enum Command {
    EnableGrayScaleTable,
    SetColumnAddress {
        start: u8,
        end: u8,
    },
    WriteRam,
    ReadRam,
    SetRowAddress {
        start: u8,
        end: u8,
    },
    SetRemapMode {
        address_increment: AddressIncrement,
        column_address_remap: bool,
        nibble_remap: bool,
        scan_direction: ScanDirection,
        split_odd_even: bool,
        dual_com_line: bool,
    },
    SetStartLine(u8),
    SetOffset(u8),
    SetMode(DisplayMode),
    EnablePartialDisplay {
        start: u8,
        end: u8,
    },
    ExitPartialDisplay,
    VddFunctionSelect(VddMode),
    SetDisplayOn(bool),
    SetPhaseLength {
        phase_1_period: u8,
        phase_2_period: u8,
    },
    SetClockDivAndOscFreq {
        clock_div: ClockDivide,
        // todo no docs on osc freq?
        osc_freq: u8,
    },
    DisplayEnhancementA(VslMode, GsDisplayQuality),
    SetGpio {
        gpio_0: GpioMode,
        gpio_1: GpioMode,
    },
    SetSecondPrechargePeriod(u8),
    // TODO this will make struct phat?
    SetGrayScaleTable([u8; 16]),
    SetDefaultGrayScaleTable,
    SetPrechargeVoltage(u8),
    SetVcomh(u8),
    SetContrastCurrent(u8),
    MasterContrastCurrentControl(u8),
    SetMuxRatio(u8),
    SetCommandLock(bool),
}

pub enum AddressIncrement {
    Horizontal = 0,
    Vertical = 1,
}

pub enum ScanDirection {
    Upwards = 0,
    Downwards = 1,
}

pub enum DisplayMode {
    AllOff = 4,
    AllOn = 5,
    Normal = 6,
    Inverse = 7,
}

pub enum VddMode {
    External = 0,
    Internal = 1,
}

pub enum ClockDivide {
    DivBy1 = 0,
    DivBy2 = 1,
    DivBy4 = 2,
    DivBy8 = 3,
    DivBy16 = 4,
    DivBy32 = 5,
    DivBy64 = 6,
    DivBy128 = 7,
    DivBy256 = 8,
    DivBy512 = 9,
    DivBy1024 = 10,
}

pub enum VslMode {
    External = 0,
    Internal = 2,
}

pub enum GsDisplayQuality {
    Normal = 0b10110,
    Enhanced = 0b11111,
}

pub enum GpioMode {
    HighZInputDisabled = 0,
    HighZInputEnabled = 1,
    OutputLow = 2,
    OutputHigh = 3,
}