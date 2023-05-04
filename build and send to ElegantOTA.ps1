pio run

Write-Host ""

# Now upload it
$hostAddress = "192.168.1.1"

$realCurlPath = "D:/source/repos/PockyBum522 Github/mqtt-temperature-humidity-reporter/buildScripts/realCurl/bin/curl.exe";

$newFirmwarePath = "D:/source/repos/PockyBum522 Github/mqtt-temperature-humidity-reporter/.pio/build/wt32-eth01/firmware.bin"

$md5 = (Get-FileHash $($newFirmwarePath) -Algorithm MD5).Hash

$mode = "firmware"

Write-Host ""
Write-Host "Posting: $($newFirmwarePath)"
Write-Host ""

# Send it!
."$($realCurlPath)" --digest --compressed -L -X POST -F "MD5=$($md5)" -F "name=$($mode)" -F "data=@$($newFirmwarePath);filename=$($mode)" "$($hostAddress)/update"
