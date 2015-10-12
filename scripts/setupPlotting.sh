echo "Exporting Python Path for Plotting"
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
export PYTHONPATH=$DIR/plotting:$PYTHONPATH
