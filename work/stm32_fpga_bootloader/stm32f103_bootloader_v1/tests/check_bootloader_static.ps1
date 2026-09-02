$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $PSScriptRoot
$bootHeader = Get-Content (Join-Path $projectRoot 'Core\Inc\bootloader.h') -Raw
$configPath = Join-Path $projectRoot 'Core\Inc\bootloader_config.h'
$bootSource = Get-Content (Join-Path $projectRoot 'Core\Src\bootloader.c') -Raw
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

Assert-True (Test-Path $configPath) 'bootloader_config.h must contain feature switches.'

if (Test-Path $configPath) {
    $config = Get-Content $configPath -Raw
    foreach ($port in 1..3) {
        Assert-True ($config -match "BOOTLOADER_ENABLE_USART$port") "USART$port enable macro is missing."
    }
    Assert-True ($config -match 'BOOTLOADER_ENABLE_GOWIN_FPGA\s+0') 'Gowin FPGA support must default to disabled.'
    Assert-True ($config -match '#error[\s\S]*USART') 'Disabling all USART ports must produce a compile-time error.'
}

foreach ($port in 1..3) {
    Assert-True ($mainSource -match "(?s)#if\s+BOOTLOADER_ENABLE_USART$port.*?MX_USART${port}_UART_Init\(\).*?#endif") "USART$port initialization is not compile-time gated."
    Assert-True ($irqSource -match "(?s)#if\s+BOOTLOADER_ENABLE_USART$port.*?void\s+USART${port}_IRQHandler\s*\(void\).*?uart_interrupt_handle\($port\).*?#endif") "USART$port IRQ is not connected and compile-time gated."
    Assert-True ($bootSource -match "BOOTLOADER_ENABLE_USART$port") "USART$port is missing from the Bootloader transport table."
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
    Assert-True ($dmaSource -match "BOOTLOADER_ENABLE_USART$port") "USART$port DMA IRQ setup is not compile-time gated."
}

if ($failures.Count -ne 0) {
    foreach ($failure in $failures) {
        Write-Error $failure -ErrorAction Continue
    }
    Write-Host "Static checks failed: $($failures.Count)"
    exit 1
}

Write-Host 'Static checks passed: 25 assertions'
