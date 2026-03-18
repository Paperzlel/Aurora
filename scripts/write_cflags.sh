var=""
last=""
for last; 
    do true; 
done

# Output command-line args into a file. 
for arg in $@; do
    if [ $arg = "-T" ]; then
        var=$var$arg" "
    else
        if [ $arg = $last ]; then 
            var=$var$arg
        else 
            var=$var$arg"\\n"
        fi;
    fi;
done

echo $var > compile_flags.txt
