$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $PSScriptRoot
$bootHeader = Get-Content (Join-Path $projectRoot 'Core\Inc\bootloader.h') -Raw
$configPath = Join-Path $projectRoot 'Core\Inc\bootloader_config.h'
$bootSource = Get-Content (Join-Path $projectRoot 'Core\Src\bootloader.c') -Raw
$cmdStart = $bootSource.IndexOf('static void CMD_40_handle')
$cmdEnd = $bootSource.IndexOf('void fpga_update_task', $cmdStart)
$cmdSource = if (($cmdStart -ge 0) -and ($cmdEnd -gt $cmdStart)) { $bootSource.Substring($cmdStart, $cmdEnd - $cmdStart) } else { '' }
$mainSource = Get-Content (Join-Path $projectRoot 'Core\Src\main.c') -Raw
$dmaSource = Get-Content (Join-Path $projectRoot 'Core\Src\dma.c') -Raw
$irqSource = Get-Content (Join-Path $projectRoot 'Core\Src\stm32f1xx_it.c') -Raw

$failures = [System.Collections.Generic.List[string]]::new()

function Assert-True {
    param(
        [bool]$Condition,
        [string]$Message
    )

    if (-not $Condition) {
        $script:failures.Add($Message)
    }
}

Assert-True (-not (Test-Path $configPath)) 'bootloader_config.h must be removed; configure bootloader.h instead.'

foreach ($port in 1..3) {
    Assert-True ($bootHeader -match "BL_ENABLE_USART$port") "USART$port enable macro is missing from bootloader.h."
}
Assert-True ($bootHeader -match 'BL_ENABLE_GOWIN_FPGA') 'Gowin FPGA configuration must be in bootloader.h.'
Assert-True ($bootHeader -match 'BL_ENABLE_ANLOGIC_FPGA') 'Anlogic FPGA configuration must be in bootloader.h.'
Assert-True ($bootHeader -match '(?s)#if\s+\(?BL_ENABLE_GOWIN_FPGA\)?\s*&&\s*\\?\s*\(?BL_ENABLE_ANLOGIC_FPGA\)?.*?#error') 'Gowin and Anlogic FPGA support must be compile-time exclusive in bootloader.h.'
Assert-True (($bootHeader + $bootSource + $mainSource + $dmaSource + $irqSource) -notmatch 'BOOTLOADER_ENABLE_') 'The old BOOTLOADER_ENABLE_ macro names must be removed.'
Assert-True ($bootHeader -match '#error[\s\S]*USART') 'Disabling all USART ports must produce a compile-time error in bootloader.h.'

Assert-True ($bootHeader -match '(?s)#if\s+\(?BL_ENABLE_GOWIN_FPGA\s*\|\|\s*BL_ENABLE_ANLOGIC_FPGA\)?.*?#define\s+BUFFER_SIZE\s+2148') 'Either FPGA backend must select a 2148-byte UART buffer.'
Assert-True ($bootHeader -match '#define\s+APPLICATION_ADDRESS_A\s+0x08003800U') 'Application address must remain 0x08003800.'
Assert-True ($bootHeader -match '#define\s+BOOT_ADDRESS\s+0x08003400U') 'Boot flag address must remain 0x08003400.'

Assert-True ($bootSource -match '#if\s+BL_ENABLE_ANLOGIC_FPGA') 'Anlogic implementation must be compile-time guarded.'
Assert-True ($bootSource -match '#if\s+BL_ENABLE_GOWIN_FPGA') 'Gowin implementation guard must be preserved.'
Assert-True ($bootSource -match '#define\s+ANLOGIC_FLASH_BASE_ADDRESS\s+0x000C0000U') 'Anlogic flash region must start at 0x000C0000.'
Assert-True ($bootSource -match '#define\s+ANLOGIC_FLASH_BLOCK_COUNT\s+12U') 'Anlogic flash region must contain twelve blocks.'
Assert-True ($bootSource -match '#define\s+ANLOGIC_FLASH_BLOCK_SIZE\s+\(64U\s*\*\s*1024U\)') 'Anlogic flash erase block must be 64 KiB.'
Assert-True ($bootSource -match '#define\s+ANLOGIC_CONTROL_DELAY_US\s+3U') 'Anlogic FPGA control delay must default to 3 us.'
Assert-True ($bootSource -notmatch 'ANLOGIC_(PAGE|BLOCK)_TIMEOUT_MS') 'Anlogic W25Q operations must not add timeout behavior absent from Boot_STM32_anlu.'
Assert-True ($bootSource -notmatch 'ANLOGIC_RELOAD_RETRY_COUNT') 'Anlogic FPGA reload must not add retry behavior absent from Boot_STM32_anlu.'
Assert-True ($bootSource -match 'GPIO_PIN_0[\s\S]*GPIO_PIN_1[\s\S]*GPIO_PIN_13[\s\S]*GPIO_PIN_9[\s\S]*GPIO_PIN_14[\s\S]*GPIO_PIN_15[\s\S]*GPIO_PIN_0') 'Anlogic backend must define the approved CS/MOSI/SCK/MISO/reset/data/ack pins.'
Assert-True ($bootSource -match '0xFFF2U?[\s\S]*0x0CU?[\s\S]*0xFFF1U?[\s\S]*0x00U?') 'Anlogic reload must select image 0x0C and issue reset.'
Assert-True ($bootSource -match '(?s)for\(bit\s*=\s*0U;\s*bit\s*<\s*16U;\s*bit\+\+\).*?\n\s*\}\s*\r?\n\s*delay_us\(ANLOGIC_CONTROL_DELAY_US\);\s*\r?\n\s*acknowledged\s*=') 'Anlogic control frames must settle after the final address clock before sampling ACK.'
Assert-True ($bootSource -notmatch 'ANLOGIC_FLASH_MAX_SIZE') 'Anlogic path must not add the source-incompatible image-capacity rejection.'
Assert-True ($bootSource -notmatch 'expected_packet') 'Anlogic path must not add strict sequential-packet state absent from Boot_STM32_anlu.'
Assert-True ($cmdSource -notmatch 'spi_w_handle\.size_t\s*==\s*0U') 'Anlogic protocol must accept the same size field semantics as Boot_STM32_anlu.'
Assert-True ($cmdSource -notmatch 'spi_w_handle\.addr\s*>\s*spi_w_handle\.size_t') 'Anlogic protocol must accept the same packet-address semantics as Boot_STM32_anlu.'
Assert-True ($cmdSource -notmatch 'spi_w_handle\.block_size_t\s*!=\s*0') 'Anlogic command length dispatch must match Boot_STM32_anlu without an extra block-size guard.'
Assert-True ($cmdSource -notmatch 'spi_w_handle\.up_cmd\s*=\s*0xff') 'Anlogic protocol errors must not manufacture the source-incompatible up_cmd=0xff state.'
Assert-True ($bootSource -match '(?s)0xfaU?[\s\S]*0x68U?[\s\S]*0x01U?[\s\S]*0x16U?') 'Anlogic FPGA entry must send the source-compatible FA 68 01 16 handshake.'
Assert-True ($bootSource -match '(?s)HAL_Delay\(1000U?\);\s*HAL_Delay\(1000U?\);') 'Anlogic FPGA entry must retain the source-compatible two-second settling delay.'
Assert-True ($bootSource -match 'ANLOGIC_W25Q_PAGE_PROGRAM') 'Anlogic backend must use W25Q page-program commands.'
Assert-True ($bootSource -notmatch 'static\s+void\s+anlogic_flash_read\s*\(') 'Anlogic packet writes must not add the disabled source readback verification pass.'
Assert-True ($dmaSource -match '#include\s+"bootloader\.h"') 'DMA source must include bootloader.h directly.'
Assert-True ($dmaSource -notmatch 'bootloader_config\.h') 'DMA source must not depend on the removed compatibility header.'

foreach ($port in 1..3) {
    Assert-True ($mainSource -match "(?s)#if\s+BL_ENABLE_USART$port.*?MX_USART${port}_UART_Init\(\).*?#endif") "USART$port initialization is not compile-time gated."
    Assert-True ($irqSource -match "(?s)#if\s+BL_ENABLE_USART$port.*?void\s+USART${port}_IRQHandler\s*\(void\).*?uart_interrupt_handle\($port\).*?#endif") "USART$port IRQ is not connected and compile-time gated."
    Assert-True ($bootSource -match "BL_ENABLE_USART$port") "USART$port is missing from the Bootloader transport table."
}

Assert-True ($bootSource -match 'typedef\s+struct[\s\S]*boot_uart_context_t') 'A persistent UART context type is required.'
Assert-True ($bootSource -match 'volatile\s+uint16_t\s+rx_length') 'RX length must be a volatile 16-bit value.'
Assert-True ($bootSource -match 'volatile\s+uint8_t\s+rx_ready') 'RX-ready state must be volatile.'
Assert-True ($bootSource -match 'uint8_t\s+tx_buffer\s*\[') 'Each UART context needs a persistent DMA TX buffer.'
Assert-True ($bootSource -match 'HAL_UART_Transmit_DMA\s*\([^,]+,\s*uart->tx_buffer') 'DMA transmission must use the persistent UART TX buffer.'
Assert-True ($bootSource -match 'void\s+my_memset\s*\([^,]+,[^,]+,\s*uint16_t\s+len\s*\)') 'my_memset length must be uint16_t.'
Assert-True ($bootSource -notmatch '\bdelay_ms\s*\(') 'The busy-wait delay must be removed from the receive path.'
Assert-True ($bootSource -notmatch '\bHAL_UART_DMAStop\s*\(') 'Frame reception must not stop TX DMA.'

$legacyPattern = '\b(URAT_NUMBER|UART_SUM|RE_UART|UART_4|huart4|hdma_uart4_rx|dma_ms|dma_ms_flag|dma_number|bsp_uart_baud_reinit)\b'
Assert-True (($bootHeader + $bootSource) -notmatch $legacyPattern) 'Legacy single-UART/UART4/unused symbols remain.'

foreach ($port in 1..3) {
    Assert-True ($dmaSource -match "BL_ENABLE_USART$port") "USART$port DMA IRQ setup is not compile-time gated."
}

if ($failures.Count -ne 0) {
    foreach ($failure in $failures) {
        Write-Error $failure -ErrorAction Continue
    }
    Write-Host "Static checks failed: $($failures.Count)"
    exit 1
}

Write-Host 'Static checks passed: 56 assertions'
