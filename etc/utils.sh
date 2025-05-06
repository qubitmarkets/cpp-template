
function run {
    echo "+ $@" >&2
    "$@"
}

function check_sudo {
    if [[ $UID != 0 ]]; then
      echo "Checking for required sudo permissions..."
      sudo echo "sudo check : Passed" || (echo "ERROR: No sudo access" && exit 1)
    fi
}
