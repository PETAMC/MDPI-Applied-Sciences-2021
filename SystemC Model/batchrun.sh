#!/usr/bin/env bash

SystemCDirectories=(
"/opt/systemc"
"/opt/systemc-2.3.3"
"/usr/local/systemc-2.3.3"
)
TotalSamples=1000000
VCPUs=$(nproc)
Host="$(hostname)"
Instances=$(( VCPUs / 2))
SamplesPerInstance=$(( TotalSamples / Instances ))
Experiments=(
mdpi-Sobel2-CA1Average
mdpi-Sobel2-CA1Gaussian
mdpi-Sobel2-CA1KDE
mdpi-Sobel2-CA2Average
mdpi-Sobel2-CA2Gaussian
mdpi-Sobel2-CA2KDE
mdpi-Sobel2-CA4Average
mdpi-Sobel2-CA4Gaussian
mdpi-Sobel2-CA4KDE
mdpi-Sobel2-TL1Average
mdpi-Sobel2-TL1Gaussian
mdpi-Sobel2-TL1KDE
mdpi-Sobel2-TL2Average
mdpi-Sobel2-TL2Gaussian
mdpi-Sobel2-TL2KDE
mdpi-Sobel2-TL4Average
mdpi-Sobel2-TL4Gaussian
mdpi-Sobel2-TL4KDE
mdpi-Sobel2-ML1Average
mdpi-Sobel2-ML1Gaussian
mdpi-Sobel2-ML1KDE
mdpi-Sobel2-ML2Average
mdpi-Sobel2-ML2Gaussian
mdpi-Sobel2-ML2KDE
mdpi-Sobel2-ML4Gaussian
mdpi-Sobel2-ML4Average
mdpi-Sobel2-ML4KDE
mdpi-JPEG-CA1Average
mdpi-JPEG-CA1Gaussian
mdpi-JPEG-CA1KDE
mdpi-JPEG-CA3Average
mdpi-JPEG-CA3Gaussian
mdpi-JPEG-CA3KDE
mdpi-JPEG-CA7Average
mdpi-JPEG-CA7Gaussian
mdpi-JPEG-CA7KDE
mdpi-JPEG-TL1Average
mdpi-JPEG-TL1Gaussian
mdpi-JPEG-TL1KDE
mdpi-JPEG-TL3Average
mdpi-JPEG-TL3Gaussian
mdpi-JPEG-TL3KDE
mdpi-JPEG-TL7Average
mdpi-JPEG-TL7Gaussian
mdpi-JPEG-TL7KDE
mdpi-JPEG-ML1Average
mdpi-JPEG-ML1Gaussian
mdpi-JPEG-ML1KDE
mdpi-JPEG-ML3Average
mdpi-JPEG-ML3Gaussian
mdpi-JPEG-ML3KDE
mdpi-JPEG-ML7Average
mdpi-JPEG-ML7Gaussian
mdpi-JPEG-ML7KDE)


# Finding SystemC-Installation
echo -e -n "\e[1;34mChecking for SystemC: "

for directory in "${SystemCDirectories[@]}" ;
do
    if [[ -d "$directory/lib-linux64" ]] ; then
        echo -e "\e[1;32mFound\e[1;30m at $directory"
        LD_LIBRARY_PATH="$directory/lib-linux64"
        break
    fi
done
if [[ -z "$SYSTEMC" ]] ; then
    echo -e "\e[1;31mNot found!"
    exit 1
fi



function ExecuteSimulation
{
    local Experiment=$1
    local ExperimentDirectory=$2

    for i in $(seq 0 $(( Instances - 1 ))) ; do
        local Offset=$(( SamplesPerInstance * i ))
        local ResultsPath=$ExperimentDirectory/samples-$(printf "%02d" $i).txt

        ./model --experiment $Experiment --iterations $SamplesPerInstance --skip $Offset 2> /dev/null > "$ResultsPath" &
    done

    # Wait until all processes finish
    wait
}



function RunExperiment
{
    local Experiment=$1
    local ExperimentDirectory="./results/${Host}/${Experiment}"
    local InfoFile="$ExperimentDirectory/infos.txt"
    mkdir -p "$ExperimentDirectory"

    echo "Date $(LANG=en_US date)"                >  $InfoFile
    echo "Instances $Instances"                   >> $InfoFile
    echo "SamplesPerInstance $SamplesPerInstance" >> $InfoFile
    echo "AllVCPUs $(nproc --all)"                >> $InfoFile
    echo "UsedVCPUs $VCPUs"                       >> $InfoFile

    LANG=en_US date > "$ExperimentDirectory/begin.txt"

    ( time ExecuteSimulation $Experiment $ExperimentDirectory ) 2> "$ExperimentDirectory/exectime.txt"

    LANG=en_US date > "$ExperimentDirectory/end.txt"

    # Merge samples of each instance
    local AllSamplesFile="$ExperimentDirectory/samples.txt"

    if [ -f "$AllSamplesFile" ] ; then
        # In case there is an old samples.txt, remove it to not append new data to old data
        rm $AllSamplesFile
    fi

    for i in $(seq 0 $(( Instances - 1 ))) ; do
        local SamplesPath=$ExperimentDirectory/samples-$(printf "%02d" $i).txt

        if [ ! -f $SamplesPath ] ; then
            echo "ERROR: Samples for run $i ($SamplesPath) does not exist!" | tee -a $InfoFile
            continue
        fi

        cat $SamplesPath >> $AllSamplesFile
        rm  $SamplesPath
    done

    # Check if run was successful
    local NumSamples=$(wc -l $AllSamplesFile | cut -d ' ' -f 1)
    if [ "$TotalSamples" != "$NumSamples" ] ; then
        echo "ERROR: Total Samples ($TotalSamples) ≠ Simulated Samples ($NumSamples)!" | tee -a $InfoFile
    fi

}



for Experiment in ${Experiments[@]} ; do
    RunExperiment $Experiment
done


# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

