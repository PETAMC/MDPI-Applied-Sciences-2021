#!/usr/bin/env bash

function GetInformation {
    # $1:
    # Sobel-TL1, JPEG-ML3, ...
    # $2: Measured delays

    local AVGDir=./mdpi-${1}Average
    local GaussDir=./mdpi-${1}Gaussian
    local KDEDir=./mdpi-${1}KDE

    echo -e "\e[1;37m$1"
    local Measured=$(./avg.py "$2")
    echo -e "\e[1;34mMeasured:    \e[1;32m$Measured"
    
    if [[ -d "$AVGDir" ]] ; then
        local Average=$(./avg.py $AVGDir/samples.txt)
        echo -e -n "\e[1;34mAVG-Model:   \e[1;36m$Average "
        echo -e -n "\e[1;31m$(octave --eval "disp((($Average -$Measured)/$Measured)*100)")% "
        echo -e -n "\e[1;35m$(pdfcompare "$2" $AVGDir/samples.txt 2> /dev/null)"
        echo
    fi
    
    if [[ -d "$GaussDir" ]] ; then
        local Gaussian=$(./avg.py $GaussDir/samples.txt)
        echo -e -n "\e[1;34mGauss-Model: \e[1;36m$Gaussian "
        echo -e -n "\e[1;31m$(octave --eval "disp((($Gaussian-$Measured)/$Measured)*100)")% "
        echo -e -n "\e[1;35m$(pdfcompare "$2" $GaussDir/samples.txt 2> /dev/null)"
        echo
    fi

    if [[ -d "$KDEDir" ]] ; then
        local KDE=$(./avg.py $KDEDir/samples.txt)
        echo -e -n "\e[1;34mKDE-Model:   \e[1;36m$KDE "
        echo -e -n "\e[1;31m$(octave --eval "disp((($KDE     -$Measured)/$Measured)*100)")% "
        echo -e -n "\e[1;35m$(pdfcompare "$2" $KDEDir/samples.txt 2> /dev/null) "
        echo -e -n "\e[1;30m$(grep real $KDEDir/exectime.txt)"
        echo
    fi
}


GetInformation JPEG-CA1 "../PlatformV2/timings/jpeg/bram/Iterations/JPEG-1Core.txt"
GetInformation JPEG-TL1 "../PlatformV2/timings/jpeg/bram/Iterations/JPEG-1Core.txt"
GetInformation JPEG-ML1 "../PlatformV2/timings/jpeg/bram/Iterations/JPEG-1Core.txt"
GetInformation JPEG-CA3 "../PlatformV2/timings/jpeg/bram/Iterations/JPEG-3Cores.txt"
GetInformation JPEG-TL3 "../PlatformV2/timings/jpeg/bram/Iterations/JPEG-3Cores.txt"
GetInformation JPEG-ML3 "../PlatformV2/timings/jpeg/bram/Iterations/JPEG-3Cores.txt"
GetInformation JPEG-CA7 "../PlatformV2/timings/jpeg/bram/Iterations/JPEG-7Cores.txt"
GetInformation JPEG-TL7 "../PlatformV2/timings/jpeg/bram/Iterations/JPEG-7Cores.txt"
GetInformation JPEG-ML7 "../PlatformV2/timings/jpeg/bram/Iterations/JPEG-7Cores.txt"

GetInformation Sobel2-CA1 "../PlatformV2/timings/sobel2/bram/Iterations/Sobel-1Core.txt"
GetInformation Sobel2-TL1 "../PlatformV2/timings/sobel2/bram/Iterations/Sobel-1Core.txt"
GetInformation Sobel2-ML1 "../PlatformV2/timings/sobel2/bram/Iterations/Sobel-1Core.txt"
GetInformation Sobel2-CA2 "../PlatformV2/timings/sobel2/bram/Iterations/Sobel-2Cores.txt"
GetInformation Sobel2-TL2 "../PlatformV2/timings/sobel2/bram/Iterations/Sobel-2Cores.txt"
GetInformation Sobel2-ML2 "../PlatformV2/timings/sobel2/bram/Iterations/Sobel-2Cores.txt"
GetInformation Sobel2-CA4 "../PlatformV2/timings/sobel2/bram/Iterations/Sobel-4Cores.txt"
GetInformation Sobel2-TL4 "../PlatformV2/timings/sobel2/bram/Iterations/Sobel-4Cores.txt"
GetInformation Sobel2-ML4 "../PlatformV2/timings/sobel2/bram/Iterations/Sobel-4Cores.txt"


