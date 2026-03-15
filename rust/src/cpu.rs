/// CPU simulator
use rand::Rng;

use crate::constants::{Mode, ProgramLoad, StackBase};
use crate::error::{CpuError, Result};
use crate::memory::Memory;
use crate::opcode::Opcode;

pub struct Cpu {
    pub memory: Memory,
    pub timer_interval: usize,

    // Registers
    pub ir: Option<Opcode>,
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
            ir: None,
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
        self.ir = None;
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
                Ok(false) => return Ok(()),
                Ok(true) => continue,
                Err(e) => return Err(e),
            }
        }
    }

    /// Execute one instruction. Returns Ok(true) to continue, Ok(false) on END.
    pub fn step(&mut self) -> Result<bool> {
        if self.fire_timer() {
            self.do_interrupt(ProgramLoad::Timer)?;
        }

        let raw = self.load(self.pc)?;
        let opcode = Opcode::try_from(raw)?;
        self.ir = Some(opcode);

        let operand = if opcode.has_operand() {
            Some(self.load(self.pc + 1)?)
        } else {
            None
        };

        let continuing = self.execute(opcode, operand)?;

        self.cycles += 1;

        // Advance PC past this instruction unless it was a CTI
        if continuing && !opcode.is_cti() {
            self.pc += if operand.is_some() { 2 } else { 1 };
        }

        Ok(continuing)
    }

    fn fire_timer(&self) -> bool {
        self.cycles > 0
            && self.timer_interval > 0
            && self.cycles % self.timer_interval == 0
    }

    fn load(&self, address: usize) -> Result<i32> {
        if self.mode == Mode::User && address > StackBase::User as usize {
            return Err(CpuError::SegmentationFault(format!("load from {address}")));
        }
        self.memory.read(address)
    }

    fn store(&mut self, address: usize, value: i32) -> Result<()> {
        if self.mode == Mode::User && address > StackBase::User as usize {
            return Err(CpuError::SegmentationFault(format!("store to {address}")));
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

    /// Execute a decoded instruction. Returns false on END.
    fn execute(&mut self, opcode: Opcode, operand: Option<i32>) -> Result<bool> {
        match opcode {
            Opcode::LoadV => {
                self.ac = operand.unwrap();
            }
            Opcode::LoadA => {
                self.ac = self.load(operand.unwrap() as usize)?;
            }
            Opcode::LoadI => {
                let addr = self.load(operand.unwrap() as usize)?;
                self.ac = self.load(addr as usize)?;
            }
            Opcode::LoadX => {
                self.ac = self.load((operand.unwrap() + self.x) as usize)?;
            }
            Opcode::LoadY => {
                self.ac = self.load((operand.unwrap() + self.y) as usize)?;
            }
            Opcode::LoadSpX => {
                self.ac = self.load(self.sp + self.x as usize)?;
            }
            Opcode::Store => {
                self.store(operand.unwrap() as usize, self.ac)?;
            }
            Opcode::Get => {
                self.ac = rand::rng().random_range(1..=100);
            }
            Opcode::Put => {
                match operand.unwrap() {
                    1 => print!("{}", self.ac),
                    2 => print!("{}", char::from(self.ac as u8)),
                    p => return Err(CpuError::InvalidOperand(format!("unknown port={p}"))),
                }
            }
            Opcode::AddX       => { self.ac += self.x; }
            Opcode::AddY       => { self.ac += self.y; }
            Opcode::SubX       => { self.ac -= self.x; }
            Opcode::SubY       => { self.ac -= self.y; }
            Opcode::CopyToX    => { self.x = self.ac; }
            Opcode::CopyFromX  => { self.ac = self.x; }
            Opcode::CopyToY    => { self.y = self.ac; }
            Opcode::CopyFromY  => { self.ac = self.y; }
            Opcode::CopyToSp   => { self.sp = self.ac as usize; }
            Opcode::CopyFromSp => { self.ac = self.sp as i32; }
            Opcode::Jump => {
                self.pc = operand.unwrap() as usize;
            }
            Opcode::JumpEq => {
                if self.ac == 0 {
                    self.pc = operand.unwrap() as usize;
                } else {
                    self.pc += 2;
                }
            }
            Opcode::JumpNe => {
                if self.ac != 0 {
                    self.pc = operand.unwrap() as usize;
                } else {
                    self.pc += 2;
                }
            }
            Opcode::Call => {
                self.push(self.pc as i32 + 2)?;
                self.pc = operand.unwrap() as usize;
            }
            Opcode::Ret => {
                self.pc = self.pop()? as usize;
            }
            Opcode::IncX => { self.x += 1; }
            Opcode::DecX => { self.x -= 1; }
            Opcode::Push => { self.push(self.ac)?; }
            Opcode::Pop  => { self.ac = self.pop()?; }
            Opcode::Interrupt => {
                self.do_interrupt(ProgramLoad::Interrupt)?;
            }
            Opcode::IReturn => {
                self.ireturn()?;
            }
            Opcode::End => {
                return Ok(false);
            }
        }
        Ok(true)
    }

    fn do_interrupt(&mut self, target: ProgramLoad) -> Result<()> {
        if !self.interrupts_enabled {
            self.pc += 1;
            return Ok(());
        }

        self.mode = Mode::System;
        self.interrupts_enabled = false;

        let u_sp = self.sp;
        let mut u_pc = self.pc;

        if target == ProgramLoad::Interrupt {
            u_pc += 1;
        }

        self.sp = StackBase::System as usize;
        self.push(u_sp as i32)?;
        self.push(u_pc as i32)?;

        self.pc = target as usize;
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
