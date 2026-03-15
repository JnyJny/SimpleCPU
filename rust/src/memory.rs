/// Memory subsystem for the SimpleCPU
use std::path::Path;

use crate::constants::{MAGIC, NWORDS};
use crate::error::{CpuError, Result};

pub struct Memory {
    words: Vec<i32>,
}

impl Memory {
    pub fn new() -> Self {
        Self {
            words: vec![0i32; NWORDS],
        }
    }

    pub fn with_data(data: &[i32]) -> Result<Self> {
        if data.len() != NWORDS {
            return Err(CpuError::ObjectFormat(format!(
                "Initializer mismatch: expected {} words, got {}",
                NWORDS,
                data.len()
            )));
        }
        Ok(Self {
            words: data.to_vec(),
        })
    }

    /// Load a binary object file (MAGIC header + i32 array)
    pub fn from_file(path: &Path) -> Result<Self> {
        let buf = std::fs::read(path)?;
        if buf.len() < 4 {
            return Err(CpuError::ObjectFormat("File too small".into()));
        }

        let magic = u32::from_be_bytes([buf[0], buf[1], buf[2], buf[3]]);
        if magic != MAGIC {
            return Err(CpuError::ObjectFormat(format!(
                "Bad magic: {magic} != {MAGIC}"
            )));
        }

        let data_bytes = &buf[4..];
        if data_bytes.len() % 4 != 0 {
            return Err(CpuError::ObjectFormat("Data not aligned to 4 bytes".into()));
        }

        let nwords = data_bytes.len() / 4;
        let mut words = vec![0i32; nwords];
        for (i, chunk) in data_bytes.chunks_exact(4).enumerate() {
            words[i] = i32::from_ne_bytes([chunk[0], chunk[1], chunk[2], chunk[3]]);
        }

        // Pad to NWORDS if the file was smaller
        words.resize(NWORDS, 0);

        Ok(Self { words })
    }

    /// Save memory to a binary object file
    pub fn save(&self, path: &Path) -> Result<()> {
        let mut buf = Vec::with_capacity(4 + self.words.len() * 4);
        buf.extend_from_slice(&MAGIC.to_be_bytes());
        for &word in &self.words {
            buf.extend_from_slice(&word.to_ne_bytes());
        }
        std::fs::write(path, buf)?;
        Ok(())
    }

    pub fn read(&self, address: usize) -> Result<i32> {
        if address >= self.words.len() {
            return Err(CpuError::MemoryRange(address));
        }
        Ok(self.words[address])
    }

    pub fn write(&mut self, address: usize, value: i32) -> Result<()> {
        if address >= self.words.len() {
            return Err(CpuError::MemoryRange(address));
        }
        self.words[address] = value;
        Ok(())
    }

    pub fn len(&self) -> usize {
        self.words.len()
    }
}

impl Default for Memory {
    fn default() -> Self {
        Self::new()
    }
}
