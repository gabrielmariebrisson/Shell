#include "Evaluation.h"
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include<sys/wait.h>
#include <sys/time.h>
#include <string.h>
#define NCOMMANDES 4

int evaluer_expr(Expression *e)
{
  struct sigaction a;
  a.sa_handler=SIG_DFL;
  sigemptyset(&a.sa_mask);
  a.sa_flags=SA_RESTART;
  sigaction(SIGCHLD,NULL,NULL);
  
  if (e->type == VIDE){
    return status;
  }if (e->type == SIMPLE){
    pid_t pid =fork();
    if(pid==0){
      execvp(e->arguments[0],e->arguments);
      perror(e->arguments[0]);
      exit(status);
    }
    waitpid(-1,&status,WNOHANG);
    if(WIFEXITED(status)){
    return WEXITSTATUS(status);
  }else{
    return WTERMSIG(status)+128;
  }
    return status;

  }if (e->type == SEQUENCE){
    status=evaluer_expr(e->gauche);
    status=evaluer_expr(e->droite);
    return status;
  }if (e->type == SEQUENCE_ET){
    if(e->gauche== NULL || e->droite== NULL ){
      return status;
    }
    if((status=evaluer_expr(e->gauche))==EXIT_SUCCESS){
      status= evaluer_expr(e->droite);
      return status;
    }
    return status;
  }if (e->type == SEQUENCE_OU){
    if( e->gauche== NULL || e->droite== NULL){
      return status;
    }
    if((status=evaluer_expr(e->gauche))==EXIT_SUCCESS){
      return status;
    }
    else{
      status=evaluer_expr(e->droite);
      return status;
    }
  }if (e->type == BG){
    return status;
  }if (e->type == PIPE){ 
    int pipefd[2];
    pipe(pipefd);
    pid_t pid =fork();
    if(pid==0){
        dup2 (pipefd[1], 1);
        close(pipefd[0]);
        close(pipefd[1]);
        status =evaluer_expr(e->gauche);
        exit(status);
    }
    else {
        waitpid(-1,&status,WNOHANG);
        dup2(pipefd[0],0);
        close(pipefd[1]);
        close(pipefd[0]);
        status =evaluer_expr(e->droite);
        waitpid(-1,&status,WNOHANG);
        if(WIFEXITED(status)){
            return WEXITSTATUS(status);
        }else{
            return WTERMSIG(status)+128;
        }
    }


  }if (e->type == REDIRECTION_I){
    int entree=open(e->arguments[0],O_RDONLY);
    int d=dup2(entree,0);
    status=evaluer_expr(e->gauche);
    close(entree);
    return status;
  }
  if (e->type == REDIRECTION_O){
    int sortie=open(e->arguments[0],O_WRONLY | O_CREAT, 0600);
    int d=dup2(sortie,1);
    status=evaluer_expr(e->gauche);
    close(sortie);
    return status;
  }
  if (e->type == REDIRECTION_A){
    return status;
  }
  if (e->type == REDIRECTION_E){
    int erreur=open(e->arguments[0],O_WRONLY | O_CREAT, 0600);
    int d=dup2 (erreur,2);
    close(erreur);
    return status;
  }if (e->type == REDIRECTION_EO){
    int fd=open(e->arguments[0],O_WRONLY | O_CREAT, 0600);
    int d=dup2(fd,2);
    d=dup2(fd,1);
    status=evaluer_expr(e->gauche);
    close(fd);
    return status;
  }
  fprintf(stderr, "not yet implemented \n");
  return EXIT_FAILURE;
}
