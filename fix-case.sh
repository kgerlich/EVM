#!/bin/bash
# Fix case-sensitive header includes

files=(
    "EVMSim/Stexep.c"
    "EVMSim/stmem.c"
    "EVMSim/steacalc.c"
    "EVMSim/STFLAGS.C"
    "68230/68230.c"
    "68230/writes.c"
    "68230/port.c"
    "68681/68681.c"
    "68681/vt100.c"
    "evmrom/evmrom.c"
    "evmrom/s19.c"
    "evmrom/binary.c"
)

for file in "${files[@]}"; do
    [ ! -f "$file" ] && continue
    
    # Convert lowercase includes to uppercase
    sed -i 's/#include "stmain\.h"/#include "STMAIN.H"/g' "$file"
    sed -i 's/#include "stflags\.h"/#include "STFLAGS.H"/g' "$file"
    sed -i 's/#include "stmem\.h"/#include "STMEM.H"/g' "$file"
    sed -i 's/#include "steacalc\.h"/#include "STEACALC.H"/g' "$file"
    sed -i 's/#include "stexep\.h"/#include "STEXEP.H"/g' "$file"
    sed -i 's/#include "stcom\.h"/#include "STCOM.H"/g' "$file"
    sed -i 's/#include "ststddef\.h"/#include "STSTDDEF.H"/g' "$file"
    sed -i 's/#include "ststate\.h"/#include "STSTATE.H"/g' "$file"
    sed -i 's/#include "ststartw\.h"/#include "STSTARTW.H"/g' "$file"
    
    echo "Fixed $file"
done

echo "Case sensitivity fixes applied"
