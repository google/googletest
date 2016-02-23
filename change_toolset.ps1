Param(
    [Parameter(Mandatory=$True,Position=1)]
    [string]$toolset
)

$files = Get-ChildItem -path . -filter *.vcxproj -recurse
foreach ($file in $files) {
    $content = gc $file.fullname
    $regex = [regex]"<PlatformToolset>(v[\d\w_]+)<\/PlatformToolset>"
    $oldtoolset = $regex.matches($content)
    if ($oldtoolset.groups.count -ge 1)
    {
        "  $($file.name) ($($oldtoolset.groups[1].value) - $toolset)"
        $content = $content -replace "<PlatformToolset>(v[\d\w_]+)<\/PlatformToolset>","<PlatformToolset>$toolset</PlatformToolset>"
        $content | sc $file.fullname
    }
    else
    {
        "Toolset not set for $($file.name)"
    }
}
