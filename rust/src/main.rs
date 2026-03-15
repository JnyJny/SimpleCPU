mod assembler;
mod constants;
mod cpu;
mod error;
mod memory;
mod opcode;

use std::path::PathBuf;
use std::process;

use clap::{Parser, Subcommand};

use assembler::Assembler;
use cpu::Cpu;
use error::CpuError;
use memory::Memory;

#[derive(Parser)]
#[command(name = "simplecpu", about = "A SimpleCPU Simulator")]
struct Cli {
    #[command(subcommand)]
    command: Commands,
}

#[derive(Subcommand)]
enum Commands {
    /// Run a program (source or object file)
    Run {
        /// Path to the program file
        path: PathBuf,

        /// Timer interrupt interval in cycles
        #[arg(short, long, default_value_t = 100)]
        timer_interval: usize,
    },

    /// Assemble source to object file
    Asm {
        /// Path to the source file
        source: PathBuf,

        /// Output path (default: source with .o extension)
        #[arg(short, long)]
        output: Option<PathBuf>,
    },
}

fn main() {
    let cli = Cli::parse();

    let result = match cli.command {
        Commands::Run { path, timer_interval } => cmd_run(&path, timer_interval),
        Commands::Asm { source, output } => cmd_asm(&source, output),
    };

    if let Err(e) = result {
        eprintln!("Error: {e}");
        process::exit(1);
    }
}

fn cmd_run(path: &PathBuf, timer_interval: usize) -> error::Result<()> {
    // Try loading as object file first, fall back to assembly source
    let memory = match Memory::from_file(path) {
        Ok(mem) => mem,
        Err(CpuError::ObjectFormat(_)) => {
            Assembler::from_file(path)?.memory()
        }
        Err(e) => return Err(e),
    };

    let mut cpu = Cpu::new(memory, timer_interval);
    cpu.run()
}

fn cmd_asm(source: &PathBuf, output: Option<PathBuf>) -> error::Result<()> {
    let dest = output.unwrap_or_else(|| source.with_extension("o"));
    let asm = Assembler::from_file(source)?;
    asm.save(&dest)?;
    Ok(())
}
