/// Assembler: parse source text into Memory
use std::path::Path;

use crate::constants::ProgramLoad;
use crate::error::Result;
use crate::memory::Memory;

pub struct Assembler {
    memory: Memory,
}

impl Assembler {
    pub fn from_file(path: &Path) -> Result<Self> {
        let source = std::fs::read_to_string(path)?;
        Self::from_source(&source)
    }

    pub fn from_source(source: &str) -> Result<Self> {
        let mut memory = Memory::new();
        let mut address = ProgramLoad::User as usize;

        for line in source.lines() {
            // Strip comments (anything after //)
            let text = line.split("//").next().unwrap_or("").trim();

            if text.is_empty() {
                continue;
            }

            // Address directive: .1000
            if text.starts_with('.') {
                if let Ok(addr) = text.trim_start_matches('.').trim().parse::<usize>() {
                    address = addr;
                }
                continue;
            }

            // Try to parse the first token as an integer
            let token = text.split_whitespace().next().unwrap_or("");
            if let Ok(value) = token.parse::<i32>() {
                memory.write(address, value)?;
                address += 1;
            }
        }

        Ok(Self { memory })
    }

    pub fn memory(self) -> Memory {
        self.memory
    }

    pub fn save(&self, path: &Path) -> Result<()> {
        self.memory.save(path)
    }
}
