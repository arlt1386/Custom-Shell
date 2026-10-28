

int main(int argc, char * argv[]){
  char * cookie;

  printf("whats your favorite cookie?");
  fgets(cookie, sizeof(cookie), stdin);
  printf("i love that one too!\n");
  return 0;
}
