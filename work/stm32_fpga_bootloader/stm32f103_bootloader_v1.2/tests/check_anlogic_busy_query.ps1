$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $PSScriptRoot
$source = Get-Content (Join-Path $projectRoot 'Core\Src\bootloader.c') -Raw
$cmdStart = $source.IndexOf('static void CMD_40_handle')
$cmdEnd = $source.IndexOf('void fpga_update_task', $cmdStart)
$cmdSource = if (($cmdStart -ge 0) -and ($cmdEnd -gt $cmdStart)) { $source.Substring($cmdStart, $cmdEnd - $cmdStart) } else { '' }
$failures = [System.Collections.Generic.List[string]]::new()

function Assert-True {
    param([bool]$Condition, [string]$Message)

    if (-not $Condition) {
        $script:failures.Add($Message)
    }
}

Assert-True ($source -match '(?s)if\s*\(\s*\(\s*\(uart_buf\[0\]\s*==\s*0xfb\).*?#if\s+BL_ENABLE_ANLOGIC_FPGA\s*\|\|\s*\(spi_w_handle\.up_cmd\s*!=\s*0U\)') 'Anlogic command handling must give busy state the same priority as Boot_STM32_anlu.'
Assert-True ($source -match '(?s)void\s+uart_interrupt_handle\s*\(.*?spi_w_handle\.up_cmd\s*!=\s*0U.*?CMD_40_handle\s*\(uart\).*?boot_uart_release_frame\s*\(uart\).*?return;.*?uart->rx_ready\s*=\s*1U') 'UART ISR must execute the source-compatible busy-state reply before deferring normal command parsing.'
Assert-True ($source -match '(?s)for\(block\s*=\s*0U;\s*block\s*<\s*ANLOGIC_FLASH_BLOCK_COUNT;.*?anlogic_flash_erase_block') 'Source-compatible fixed Anlogic erase-region loop must remain.'
Assert-True ($source -match '(?s)static\s+void\s+anlogic_flash_wait_ready\s*\(\s*void\s*\).*?while\s*\(\s*\(anlogic_flash_read_status\(\)\s*&\s*ANLOGIC_W25Q_STATUS_WIP\)\s*!=\s*0U\s*\)') 'Anlogic W25Q polling must wait without a software timeout, like Boot_STM32_anlu.'
Assert-True ($source -notmatch 'ANLOGIC_(PAGE|BLOCK)_TIMEOUT_MS') 'Boot_STM32_anlu has no W25Q page or block timeout constants.'
Assert-True ($source -notmatch 'static\s+void\s+anlogic_flash_read\s*\(') 'Boot_STM32_anlu does not perform a write-readback verification pass.'
Assert-True ($source -notmatch 'expected_packet') 'Anlogic path must not add strict sequential-packet state absent from Boot_STM32_anlu.'
Assert-True ($source -notmatch 'ANLOGIC_FLASH_MAX_SIZE\s*/\s*spi_w_handle\.block_size_t') 'Anlogic path must not reject image size using an extra capacity check absent from Boot_STM32_anlu.'
Assert-True ($cmdSource -notmatch 'spi_w_handle\.size_t\s*==\s*0U') 'Anlogic protocol must accept the same size field semantics as Boot_STM32_anlu.'
Assert-True ($cmdSource -notmatch 'spi_w_handle\.addr\s*>\s*spi_w_handle\.size_t') 'Anlogic protocol must accept the same packet-address semantics as Boot_STM32_anlu.'
Assert-True ($cmdSource -notmatch 'spi_w_handle\.block_size_t\s*!=\s*0') 'Anlogic command length dispatch must match Boot_STM32_anlu without an extra block-size guard.'
Assert-True ($cmdSource -notmatch 'spi_w_handle\.up_cmd\s*=\s*0xff') 'Anlogic protocol errors must not manufacture the source-incompatible up_cmd=0xff state.'
Assert-True ($source -match '(?s)0xfaU?[\s\S]*0x68U?[\s\S]*0x01U?[\s\S]*0x16U?') 'Anlogic FPGA entry must send the source-compatible FA 68 01 16 handshake.'
Assert-True ($source -match '(?s)HAL_Delay\(1000U?\);\s*HAL_Delay\(1000U?\);') 'Anlogic FPGA entry must retain the source-compatible two-second settling delay.'

if ($failures.Count -ne 0) {
    $failures | ForEach-Object { Write-Error $_ -ErrorAction Continue }
    Write-Host "Anlogic busy-query checks failed: $($failures.Count)"
    exit 1
}

Write-Host 'Anlogic busy-query checks passed: 14 assertions'
