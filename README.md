# run wsl app

* built with vs2017.
* tested on windows 10 1709.16299

    usage:
    wslrun [OPTIONS] [cmd [args...]]
    
    OPTIONS:
      ~: run in home dir (home)
      --nopath: no nt path
      --sudo: run with root


### examples

    # vim
    wslrun vim test.c
    
    # run default shell (bash)
    wslrun
    
    # run with root
    wslrun --sudo
    
    # run sshd on login. put this code to xxx.bat in "start menu" -> "Start up" folder
    wslrun --sudo --daemon bash -c "service ssh start ; cat"
