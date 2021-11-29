export GMXRESCUE=$HOME/Library/gmx_rescue/gmx_rescue
module load gromacs/5.1.3

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

function get_last_good_frame() {
    gmx check -f $1 2>&1 > /dev/null | tail -1 \
        | sed "s|\r|\n|g" | tail -2 | head -1 \
        | tr -s " " | cut -d" " -f3,5
}

function strip_front_section() {
    LASTGOODFRAME=$(get_last_good_frame $1)
    ENDFRAME=$(echo $LASTGOODFRAME | cut -d" " -f1)
    ENDTIME=$(echo $LASTGOODFRAME | cut -d" " -f2)
    echo "Stripping at frame $ENDFRAME ($ENDTIME)"
    gmx trjconv -f $1 -o $2 -e $ENDTIME 
}

function find_next_good_frame() {
    FILENAME=$1
    CHECKFRAME=$2
    OUTNAME=$3
    TMPXTC=tmp.$2.xtc
    echo -e "${GREEN}Looking at frame $CHECKFRAME...${NC}"
    TMPSCAN=$($GMXRESCUE $FILENAME scan  | tr -s " ")
    if ( ! grep -q " $CHECKFRAME " <<< $TMPSCAN); then
        echo -e "${RED}Uh oh, cannot find the frame you wanted! ($CHECKFRAME)${NC}"
        return 1
    fi
    TMPOFFSET=$(echo $TMPSCAN | sed "s| | \n |g" \
            | grep " $CHECKFRAME " -A 2 | tail -1 | tr -d " ")
    echo -e "${GREEN}Found offset of $TMPOFFSET for frame $CHECKFRAME${NC}"
    echo -e "${GREEN}Extracting...${NC}"
    $GMXRESCUE $1 $TMPXTC $TMPOFFSET > /dev/null
    echo -e "${GREEN}Checking...${NC}"
    gmx check -f $TMPXTC && (mv $TMPXTC $OUTNAME; return 0) || (rm $TMPXTC ; find_next_good_frame $1 $(( $2 + 1 )) $OUTNAME)
}

function combine_and_check() {
    echo -e "${GREEN}Concatenating the two parts...${NC}"
    gmx trjcat -f $1 $2 -o $3 
    echo -e "${GREEN}Checking health of new trajectory...${NC}"
    gmx check $3 || (echo -e "${RED}Could not repair.${NC}" ; return 1) && (echo -e "${GREEN}Repaired.${NC}" ; return 0)
}   


