#!/bin/bash
# Recollect the arches.

if [ $# -ne 2 -o ! -d ${1} -o ! -d ${2} ]
then
    echo "Usage: `basename ${0}` path/to/arch path/to/lib"
    exit 1
fi

export LC_COLLATE=C
i=${#1}
let i=i-1

if [ ${1:${i}} == "/" ]
then
    arch=${1:0:${i}}
else
    arch=${1}
fi

let i=${#2}
let i=i-1

if [ ${2:${i}} == "/" ]
then
    lib=${2:0:${i}}
else
    lib=${2}
fi

TMPFILE="/tmp/daimonin.tmp.1"

if [ -f ${TMPFILE} ]
then
    rm ${TMPFILE}
fi

# $1 : pathname
ignore_path()
{
    local i=${#arch}
    let i=i+5 # /dev/

    if [[ ${1:0:${i}} == "${arch}/dev/" ]]
    then
        return 0
    else
        return 1
    fi
}

# $1 : nrof backspaces
print_backspaces()
{
    local backspaces=${1}

    while [ ${backspaces} -gt 0 ]
    do
        echo -n ""
        let backspaces=backspaces-1
    done
}

# $1 : Filename
# $2 : Extension
# $3 : Defn start
# $4 : Defn end
# $5 : Defn more
collect_objects()
{
    echo -n "### Collecting ${1}... (pass 1)... "
    let num=0
    count=""

    for path in `find "${arch}" -type f -name "*.${2}"`
    do
        ignore_path ${path}
    
        if [ $? -eq 0 ]
        then
            continue
        fi
    
        let num=num+1

        if [ -z "${dai_suppress_progress}" ]
        then
            print_backspaces ${#count}
            count="${num} "
            echo -n "${count}"
        fi

        more=0
    
        while read line
        do
            if [[ -n "${5}" && ${line} == "${5}" ]]
            then
                let more=1
                continue
            fi

            if [[ ${line:0:${#3}} == "${3}" ]]
            then
                if [ ${more} -eq 1 ]
                then
                    let more=0
                else
                    name=`echo "${line}" | cut -d ' ' -f 2`
                    echo "${name} ${path}" >> ${TMPFILE}
                fi
            fi
        done < ${path}
    done

    if [ -n "${dai_suppress_progress}" ]
    then
        echo -n "${num} "
    fi

    echo -n "(pass 2)... "
    tmp=`cat ${TMPFILE} | sort`
    echo "${tmp}" > ${TMPFILE}
    let num=0
    count=""
    
    if [ -f ${lib}/${1} ]
    then
        rm ${lib}/${1}
    fi
    
    while read line
    do
        name=`echo "${line}" | awk '{print $1}'`
        path=`echo "${line}" | awk '{print $2}'`
        indefn=0
        end=0
    
        while read defn
        do
            # Kill CR
            defn=${defn/%/}

            if [[ -n "${5}" && ${defn} == "${5}" ]]
            then
                let end=0
            fi

            if [ ${end} -eq 1 ]
            then
                break
            fi

            if [[ ${defn} == "${3} ${name}" ]]
            then
                let indefn=1
            fi
    
            if [ ${indefn} -eq 1 ]
            then
                echo "${defn}" >> ${lib}/${1}

                if [[ ${defn} == "${4}" ]]
                then
                    let end=1
                fi
            fi
        done < ${path}

        # Several defn files have no EOF line ending, which confuses read, so:
        if [ ${indefn} -eq 1 ] && [ ${end} -eq 0 ]
        then
            echo "${4}"  >> ${lib}/${1}
        fi

        let num=num+1

        if [ -z "${dai_suppress_progress}" ]
        then
            print_backspaces ${#count}
            count="${num} "
            echo -n "${count}"
        fi
    done < ${TMPFILE}

    if [ -n "${dai_suppress_progress}" ]
    then
        echo -n "${num} "
    fi
    
    rm ${TMPFILE}
    echo "DONE!"
}

########
# Objects

collect_objects 'animations' 'anim' 'anim' 'mina'
collect_objects 'archetypes' 'arc' 'Object' 'end' 'More'

########
# Images

echo -n "### Collecting images... (pass 1)... "
let num=0
count=""

for path in `find "${arch}" -type f -name '*.png'`
do
    ignore_path ${path}

    if [ $? -eq 0 ]
    then
        continue
    fi

    let num=num+1

    if [ -z "${dai_suppress_progress}" ]
    then
        print_backspaces ${#count}
        count="${num} "
        echo -n "${count}"
    fi

    echo "`basename ${path} '.png'` ${path}" >> ${TMPFILE}
done

if [ -n "${dai_suppress_progress}" ]
then
    echo -n "${num} "
fi

let i=num
let count=1

while [ ${i} -ne 0 ]
do
    let i=i/10
    let count=count+1
done

images=`cat ${TMPFILE} | sort`
bmaps=`echo "${images}" | awk '{print $1}' | nl -n rz -s ' ' -v 1 -w ${count}`
echo "`printf "%0${count}d" 0` bug.101" > ${lib}/bmaps
echo "${bmaps}" >> ${lib}/bmaps
echo -n "(pass 2)... "
let num=0
count=""
echo "IMAGE 0 321 bug.101" > ${lib}/daimonin.0
cat ${arch}/dev/editor/bug.101.png >> ${lib}/daimonin.0
bmaps=`echo "${images}" | awk '{print $1}'`

for  path in `echo "${images}" | awk '{print $2}'`
do
    let num=num+1

    if [ -z "${dai_suppress_progress}" ]
    then
        print_backspaces ${#count}
        count="${num} "
        echo -n "${count}"
    fi

    size=`wc -c ${path} | awk '{print $1}'`
    echo "IMAGE ${num} ${size} `basename ${path} '.png'`" >> ${lib}/daimonin.0
    cat ${path} >> ${lib}/daimonin.0
done

if [ -n "${dai_suppress_progress}" ]
then
    echo -n "${num} "
fi

echo "DONE!"
rm ${TMPFILE}
unset LC_COLLATE
