$ErrorActionPreference = "Stop"

if (!($args.Length -eq 1))
{
    Write-Warning "Please provide folder name as argument"
    Exit
}

$publishFolder = $args[0]
$publishPath = "..\4-host\$publishFolder"

if (!(test-path $publishPath))
{
    New-Item -ItemType Directory -Force -Path $publishPath
}
Remove-Item "$publishPath\*"
Copy-Item "out\*" $publishPath