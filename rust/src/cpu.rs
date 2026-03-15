/// CPU simulator
use rand::Rng;

use crate::constants::{Mode, ProgramLoad, StackBase};
use crate::error::{CpuError, Result};
use crate::memory::Memory;
use crate::opcode;

pub struct Cpu {
    pub memory: Memory,
    pub timer_interval: usize,

    // Registers
    pub ir: i32,
    pub pc: usize,
    pub sp: usize,
    pub ac: i32,
    pub x: i32,
    pub y: i32,

    // State
    pub mode: Mode,
    pub cycles: usize,
    pub interrupts_enabled: bool,
}

impl Cpu {
    pub fn new(memory: Memory, timer_interval: usize) -> Self {
        Self {
            memory,
            timer_interval,
            ir: 0,
            pc: ProgramLoad::User as usize,
            sp: StackBase::User as usize,
            ac: 0,
            x: 0,
            y: 0,
            mode: Mode::User,
            cycles: 0,
            interrupts_enabled: true,
        }
    }

    pub fn reset(&mut self) {
        self.mode = Mode::User;
        self.ir = 0;
        self.pc = ProgramLoad::User as usize;
        self.sp = StackBase::User as usize;
        self.ac = 0;
        self.x = 0;
        self.y = 0;
        self.cycles = 0;
        self.interrupts_enabled = true;
    }

    /// Run the program to completion
    pub fn run(&mut self) -> Result<()> {
        loop {
            match self.step() {
                Ok(false) => return Ok(()),  // END instruction
                Ok(true) => continue,
                Err(e) => return Err(e),
            }
        }
    }

    /// Execute one instruction. Returns Ok(true) to continue, Ok(false) on END.
    pub fn step(&mut self) -> Result<bool> {
        // Check timer interrupt
        if self.fire_timer() {
            self.interrupt(ProgramLoad::Timer)?;
        }

        // Fetch instruction
        self.ir = self.load(self.pc)?;
        let info = opcode::lookup(self.ir)?;

        // Fetch operand if needed
        let operand = if info.has_operand {
            Some(self.load(self.pc + 1)?)
        } else {
            None
        };

        // Execute
        let is_cti = info.is_cti;
        let advanced = self.execute(info.name, operand)?;

        self.cycles += 1;

        // Advance PC if the instruction didn't do it (not a CTI)
        if !is_cti && advanced {
            self.pc += if operand.is_some() { 2 } else { 1 };
        }

        Ok(advanced)
    }

    fn fire_timer(&self) -> bool {
        self.cycles > 0
            && self.timer_interval > 0
            && self.cycles % self.timer_interval == 0
    }

    fn load(&self, address: usize) -> Result<i32> {
        if self.mode == Mode::User {
            let user_end = StackBase::User as usize;
            if address > user_end {
                return Err(CpuError::SegmentationFault(format!("load from {address}")));
            }
        }
        self.memory.read(address)
    }

    fn store(&mut self, address: usize, value: i32) -> Result<()> {
        if self.mode == Mode::User {
            let user_end = StackBase::User as usize;
            if address > user_end {
                return Err(CpuError::SegmentationFault(format!("store to {address}")));
            }
        }
        self.memory.write(address, value)
    }

    fn push(&mut self, value: i32) -> Result<()> {
        self.sp -= 1;
        self.store(self.sp, value)
    }

    fn pop(&mut self) -> Result<i32> {
        let stackbase = StackBase::for_mode(self.mode) as usize;
        if self.sp >= stackbase {
            return Err(CpuError::StackUnderflow(self.sp, self.mode));
        }
        let value = self.load(self.sp)?;
        self.sp += 1;
        Ok(value)
    }

    /// Execute a single instruction by name. Returns false on END.
    fn execute(&mut self, name: &str, operand: Option<i32>) -> Result<bool> {
        match name {
            "loadv"      => { self.ac = operand.unwrap(); }
            "loada"      => { self.ac = self.load(operand.unwrap() as usize)?; }
            "loadi"      => {
                let addr = self.load(operand.unwrap() as usize)?;
                self.ac = self.load(addr as usize)?;
            }
            "loadx"      => { self.ac = self.load((operand.unwrap() + self.x) as usize)?; }
            "loady"      => { self.ac = self.load((operand.unwrap() + self.y) as usize)?; }
            "loadspx"    => { self.ac = self.load(self.sp + self.x as usize)?; }
            "store"      => { self.store(operand.unwrap() as usize, self.ac)?; }
            "get"        => { self.ac = rand::rng().random_range(1..=100); }
            "put"        => {
                match operand.unwrap() {
                    1 => print!("{}", self.ac),
                    2 => print!("{}", char::from(self.ac as u8)),
                    p => return Err(CpuError::InvalidOperand(format!("unknown port={p}"))),
                }
            }
            "addx"       => { self.ac += self.x; }
            "addy"       => { self.ac += self.y; }
            "subx"       => { self.ac -= self.x; }
            "suby"       => { self.ac -= self.y; }
            "copytox"    => { self.x = self.ac; }
            "copyfromx"  => { self.ac = self.x; }
            "copytoy"    => { self.y = self.ac; }
            "copyfromy"  => { self.ac = self.y; }
            "copytosp"   => { self.sp = self.ac as usize; }
            "copyfromsp" => { self.ac = self.sp as i32; }
            "jump"       => { self.pc = operand.unwrap() as usize; }
            "jumpeq"     => {
                if self.ac == 0 {
                    self.pc = operand.unwrap() as usize;
                } else {
                    self.pc += 2;
                }
            }
            "jumpne"     => {
                if self.ac != 0 {
                    self.pc = operand.unwrap() as usize;
                } else {
                    self.pc += 2;
                }
            }
            "call"       => {
                self.push(self.pc as i32 + 2)?;
                self.pc = operand.unwrap() as usize;
            }
            "ret"        => {
                self.pc = self.pop()? as usize;
            }
            "incx"       => { self.x += 1; }
            "decx"       => { self.x -= 1; }
            "push"       => { self.push(self.ac)?; }
            "pop"        => { self.ac = self.pop()?; }
            "interrupt"  => { self.interrupt(ProgramLoad::Interrupt)?; }
            "ireturn"    => { self.ireturn()?; }
            "end"        => { return Ok(false); }
            _            => { return Err(CpuError::MachineCheck(format!("Unknown instruction: {name}"))); }
        }
        Ok(true)
    }

    fn interrupt(&mut self, program_load: ProgramLoad) -> Result<()> {
        if !self.interrupts_enabled {
            self.pc += 1;
            return Ok(());
        }

        self.mode = Mode::System;
        self.interrupts_enabled = false;

        let u_sp = self.sp;
        let mut u_pc = self.pc;

        if program_load == ProgramLoad::Interrupt {
            u_pc += 1;
        }

        self.sp = StackBase::System as usize;
        self.push(u_sp as i32)?;
        self.push(u_pc as i32)?;

        self.pc = program_load as usize;
        Ok(())
    }

    fn ireturn(&mut self) -> Result<()> {
        self.pc = self.pop()? as usize;
        self.sp = self.pop()? as usize;
        self.interrupts_enabled = true;
        self.mode = Mode::User;
        Ok(())
    }
}
