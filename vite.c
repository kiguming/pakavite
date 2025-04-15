#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <curses.h>
#else
#include <ncurses.h>
#endif

typedef struct Node {
    char *text; //텍스트부분 
    struct Node *next; //다음노드를 가르킴 
    struct Node *prev; //이전노드를 가르킴 
} Node;

typedef struct Editor{
    Node *head; //전체의 제일 앞 
    Node *tail; //전체의 제일 뒤 
    int total_lines; //총합 라인의 갯수
    int top_line; //텍스트에디터에서 제일 최상단의 첫라인 
} Editor;

void initEditor(Editor *editor) { //구조체 초기화 
    editor->head = NULL;
    editor->tail = NULL;
    editor->total_lines = 0;
    editor->top_line = 0;
}

void addNode(Editor *editor, char* text){
	Node *newNode = (Node *)malloc(sizeof(Node)); // 새 노드 동적 할당
    newNode->text = strdup(text); // 주어진 텍스트를 복사하여 노드에 저장
    newNode->next = NULL; // 일단 초기화 
    newNode->prev = NULL; // 일단 초기화

    if (editor->head == NULL) {// 만약에 editor가 비어있다면 
        editor->head = newNode; //머리에도 새노드가 들어가고 
        editor->tail = newNode; //꼬리에도 새노드를 넣고 
    } else {
        newNode->prev = editor->tail; // 지금 가장 꼬리에있는 것을 새노드의 앞에 넣고 
        editor->tail->next = newNode; // 현재 꼬리의 다음에 새노드를 넣고 
        editor->tail = newNode; //새노드를 꼬리에 넣음 
    }
    editor->total_lines++; // 전체 라인 수 증가
}


void displayScreen(Editor* editor,Node* editNode, int term_sizeY, int term_sizeX,int curser_positionY){
	int i = 0;
	int nofindNext = 0;
	 
	Node* screenNode = editNode;
	if(screenNode->prev !=NULL){
		for(i=0;i<curser_positionY;i++){
			screenNode = screenNode->prev;//스크린노드가 커서의 Y의 최상단까지 이동시킴. 
		}
	} 
	
	
	for(i=0;i<term_sizeY-2;i++){
		if(screenNode->text != NULL && nofindNext == 0  ){//스크린노드에 텍스트 값이 있다면
		printw("%.*s\n",term_sizeX-1, screenNode->text);//출력하고 
		}else{
			printw("~\n"); //없다면 ~를 출력해라. 
		}
		if(screenNode->next != NULL){//스크린 노드의 다음이 있다면 
			screenNode = screenNode->next;//다음노드 이동해라 
		}else{
			nofindNext =1;
		} 
		
	}

}


void statusBar(int curser_positionY, int curser_positionX,char* fileName,int linesize,int term_sizeX){
	start_color();  // 색상모드 킴 
    init_pair(1, COLOR_WHITE, COLOR_BLACK);  // 흰색 배경에 검정 글자
    init_pair(2, COLOR_BLACK, COLOR_WHITE);  // 검정 배경에 흰색 글자 
	
	int size = 0;
	size += strlen(fileName); // 파일이름 사이즈 
	size += 28; //기타요소의 사이즈; 
	
	size = term_sizeX - size;//x,y출력과 파일이름 사이에 간격 
	int i=0; 
	
	attron(COLOR_PAIR(1) | A_REVERSE);
    printw("[%s] - %10d Lines",fileName,linesize);//24번째라인 상태 바
    for(i=0;i<size;i++){
    	printw(" ");//사이즈 만큼 중간 공백 출력 
	}
    printw("%3d %3d\n",curser_positionY,curser_positionX);
    attroff(COLOR_PAIR(1) | A_REVERSE);
}

void msgBar(int msg,char* searchText){
	switch(msg){
		case 0:
			printw("Help:ctrl-s = Save | ctrl-q = quit | ctrl-F = find");//25번째라인 메시지바
			break;
		case 1:
			printw("search text : %s  + enter / out mode = esc", searchText);//25번째라인 메시지바
			break;
		case 2:
			printw("saved file.");
			break;
		case 3:
			printw("saved mode. if you want to save. FIle name + enter");
			break;	
		case 4:
			printw("if you quit. one more ctrl-q.");//25번째라인 메시지바
			break;
		case 5:
			printw("no saved filename.");
			break;
		case 6:
			printw(" <-,-> enter - moved findtext.  esc - end.");
			break;
		case 7:
			printw("not founded");
			break;	
	}
}

void editLine(Node* editNode, char key,int curser_positionX){//현재라인을 수정하는 변수 
	
	int length = strlen(editNode->text);//현재 노드의 길이를 받아와서 
	editNode->text = (char*)realloc(editNode->text, length + 2); //\0도 넣어야되서 두자리추가 
	if(length == curser_positionX){//글자 맨마지막에 추가하는거 
		editNode->text[length] = key;
		editNode->text[length+1] = '\0';
	}else{
		memmove(editNode->text + curser_positionX + 1, editNode->text + curser_positionX, length - curser_positionX + 1);
		editNode->text[curser_positionX] = key;
	}

	
}

void delete(Editor* editor, Node* editNode, int curser_positionX,int curser_positionY){
	
	int length = strlen(editNode->text);//현재 노드의 사이즈 
	if(editNode->prev != NULL){//앞에 노드가 있어야되용. 
		if(curser_positionX == 0){//X가 맨 왼쪽일때
			if(editNode->text[0] == '\0'){//텍스트 내용이 없다면 
				if(editNode->next != NULL){//에딧노드 뒤에 뭐가있으면
					Node* nextNode = editNode->next; 
					editNode = editNode->prev; //에딧노드 한칸땡기고 
					free(editNode->next); //에딧노드 뒤 날리고 
					editNode->next = nextNode; 
					nextNode->prev = editNode;
				}else{//없으면 
					editNode = editNode->prev;
					editor->tail = editNode;
					free(editNode->next);
					editNode->next = NULL;
				}	
			}else{ 
				char* buffer = (char*)malloc(strlen(editNode->text));//버퍼 공간 만듬 
				strcpy(buffer,editNode->text);//버퍼에 값 임시저장할
				buffer[strlen(editNode->text)] = '\0';//여기까지 1차수정
				if(editNode->next ==NULL){//에딧노드 뒤가 없다면, 
					editNode = editNode->prev;//앞으로 이동하고 
					editor->tail = editNode;//꼬리에 지금 에딧노드를 넣고 
					free(editNode->next->text);//next를 날릴거니까 지우고 
					free(editNode->next);//next를 날릴거니까 지우고 
					editNode->next = NULL;//에딧 다음노드를 없애고 
				}else{//에딧노드 뒤에 노드가있다면 
					Node* nextNode =  editNode->next;//임시 앞 노드 지정 
					editNode =  editNode->prev; //에딧 노드를 앞노드로 가고 
					nextNode->prev = editNode; //앞노드를 next노드의 앞이라고 지정하고 
					editNode->next = nextNode; //에딧 노드에 넥스트 노드를 연결하고
					
				}
				
				editNode->text = (char*)realloc(editNode->text,strlen(editNode->text) + length);//크기 재배정 
				strcat(editNode->text, buffer);
				
				free(buffer);
			} 
		editor->total_lines--;//다 처리하고 줄한칸 내림. 	 
		}else{//X가 맨 왼쪽이 아닐때 
			memmove(editNode->text + curser_positionX - 1, editNode->text + curser_positionX, length - curser_positionX + 1);
		}
	}else{
		if(curser_positionX > 0){
			memmove(editNode->text + curser_positionX - 1, editNode->text + curser_positionX, length - curser_positionX + 1);
		}
	}
	
}

void makeMidnode(Editor* editor,Node* editNode, char* text){
	Node* newNode = (Node *)malloc(sizeof(Node));
    newNode->text = strdup(text); // 주어진 텍스트를 복사하여 노드에 저장
    newNode->next = NULL;
    newNode->prev = NULL;
	
	Node* prevNode = editNode;
	Node* nextNode = editNode->next;
	
    
    newNode->prev = prevNode;
    newNode->next = nextNode;

    if (prevNode != NULL) {
        prevNode->next = newNode;
    }
    if (nextNode != NULL) {
        nextNode->prev = newNode;
    }

    // editor의 head와 tail 포인터 업데이트 
    if (prevNode == NULL) {
        editor->head = newNode;
    }
    if (nextNode == NULL) {
        editor->tail = newNode;
    }

    // total_lines 카운트 증가
    editor->total_lines++;
    
}

void fileSave(Editor* editor,char* fileName){
	FILE* file = fopen(fileName, "w");
	Node* sNode = editor->head;
	if(file != NULL){
		while(sNode->next != NULL){ //s노드의 뒤가 없을때까지 
		char* buffer = (char*)malloc(strlen(sNode->text) + 2);  // +1하니까 글자깨짐. \n과 \0이 있어야함 
        strcpy(buffer, sNode->text);//버퍼에 텍스트 복사. 
        strcat(buffer, "\n");
        fputs(buffer, file);
        free(buffer);  // 메모리 ㅎ ㅐ제 
        sNode = sNode->next;
		}
		//마지막노드 처리 
		char* buffer = (char*)malloc(strlen(sNode->text) + 2);  // +1하니까 글자깨짐. \n과 \0이 있어야함 
        strcpy(buffer, sNode->text);
        strcat(buffer, "\n");
        fputs(buffer, file);
        free(buffer);
	}
	fclose(file);
	
	
}

int searchBoyerMooreForward(char *text, char *searchText, int start) {
    int m = strlen(searchText); //검색할 텍스트 사이즈 
    int n = strlen(text); //텍스트 전체사이즈. 
    int s = start; //텍스트에서 시작할 위치 
    while (s <= (n - m)) { //시작할위치가 남은텍스트보다 많을동안 
        int j = m - 1;
        while (j >= 0 && searchText[j] == text[s + j]) {
            j--;
        }
        if (j < 0) {//패턴이 끝까지 일치하면 
            return s;//찾은 글자 맨마지막위치의 숫자를 리턴 
        } else {
            s += 1;//그게아니면 시작위치 이동 
        }
    }

    return -1; // 못찾으면 -1 
}

int searchNext(char *text, char *searchText, int start) {
    int nextPosition = searchBoyerMooreForward(text, searchText, start + 1);
    return nextPosition;
}

int searchPrevious(char *text, char *searchText, int start) { //보이어무어 반대방향. 
    int m = strlen(searchText);
    int i = 0;
    int j = 0;
    for (i = start - 1; i >= 0; i--) { //끝부터 반대로 돌아서 
        int match = 1; 
        for (j = 0; j < m; j++) {
            if (text[i + j] != searchText[j]) {
                match = 0; 
                break;
            }
        }
        if (match == 1) {
            return i; //일치한다면 제일 앞을 리턴해줌 
        }
    }
    return -1; //못찾으면 -1을 리턴해줌 
}


int main(int argc, char *argv[]) {
	Editor editor;         //에디터 구조체 생성. 
	initEditor(&editor);   //에디터 초기화 
	
	
	
	char* fileName =malloc(1);//파일이름 부터 선언. 
	*fileName ='\0'; //파일이름 일단 없는걸로 
	if(argc == 2){//매개변수가 있다면 
		fileName = malloc(strlen(argv[1]) + 1); //매개변수 만큼 사이즈 다시 늘리고 
		strcpy(fileName, argv[1]);//fileName에 이름 넣음 
		FILE *file = fopen(argv[1], "r");//파일 읽기,쓰기 모드. 
    	if(file == NULL){//파일이없다면 
    		return 0;//그냥 바로꺼버림 
		}
    	char buffer[1024];//글 넣을 임시버퍼 
    	//수정한번해야됨. 
    	while(fgets(buffer, sizeof(buffer), file) != NULL){//파일의 포인터 검사함 
    		
    		char *newline = strchr(buffer, '\n');
	        if (newline != NULL) {
	            *newline = '\0';
	        }
    		addNode(&editor, buffer);
		}
		
    	fclose(file);
	}
	
	if(editor.head ==NULL){//노드에 아무것도 없으면 
		addNode(&editor,""); //노드 생성하기 
	}
	
	Node* editNode = editor.head;
	int editMode = 0; //에디터 모드 0이면 기본, 1이면 저장할 파일명입력, 2이면 검색모드 
	int term_sizeY = 26; //임시로 지정한 터미널 사이즈 
	int term_sizeX = 80; //임시로 지정한 터미널 사이즈 
    int curser_positionY = 0;// 커서의 Y축 값 
    int curser_positionX = 0;// 커서의 X축 값
    int changetext = 0; //텍스트가 바뀌었는지 확인하는 값
	//메세지바 출력하거나 모드 바꾸는 값 0기본값 1검색 2저장 3종료 
	int end = 1;//ctrl - q로 인해서 변화하는 값.
	int msg = 0;
    initscr();            // 커서 초기화
    raw();                // Raw 모드 설정
    keypad(stdscr, TRUE); // 특수 키 사용 가능하도록 설정
    noecho();             // 키 입력 시 문자 출력 비활성화
    curs_set(1);          // 커서 보이기
	getmaxyx(stdscr, term_sizeY, term_sizeX);
	
	char* searchText =malloc(1); 
	*searchText ='\0';
    int searchStart = 0;
    while (end) {
        clear(); // 화면 지우기
        displayScreen(&editor, editNode, term_sizeY, term_sizeX,curser_positionY);//화면보이기 
		statusBar(curser_positionY,curser_positionX,fileName,editor.total_lines,term_sizeX);
		msgBar(msg, searchText);
        move(curser_positionY, curser_positionX);
		
        int key = getch(); //키 입력
		
		
			
		if(editMode == 0){//입력모드일때
			msg = 0; 
	        // 키 처리
	        switch (key) {
	        	case KEY_PPAGE: // 페이지 업 입력 
	        	case KEY_UP: //위 입력
	        		if(curser_positionY == 0){//커서가 제일 위고 
						if(editNode->prev !=NULL){//에딧 노드 위에 있다면 
	        				editNode = editNode -> prev;//에딧노드 위로 올림 
						}//Y가 0이고 editNode도 Head라면 아무 일도 안 일어남 
					}else{
						editNode = editNode -> prev; //전칸으로 가고 
						curser_positionY--; //커서 위치도 옮기고	
					} 
					if(curser_positionX > strlen(editNode->text)){//X가 editNode 크기보다 오른쪽에 있다면 
							curser_positionX = strlen(editNode->text);//X위치도 바꿈. 
					}
	                break;
	            case KEY_NPAGE://페이지 다운입력 
	            case KEY_DOWN://아래 입력 
	            	if(editNode ->next !=NULL){//노드 밑에 뭐가있는지 부터 확인 
	            		if(curser_positionY == term_sizeY-3){//터미널사이즈-2보다 작다면 
							editNode = editNode->next;//제일 하단이니 노드만 다음노드로 보내면됨.
							 
						}else{
							curser_positionY++; //밑으로 내려가고 
							editNode = editNode->next;//노드위치도 내리고 
						}
						if(curser_positionX > strlen(editNode->text)){//X가 editNode 크기보다 오른쪽에 있다면 
							curser_positionX = strlen(editNode->text);//X위치도 바꿈. 
						}
					}
	                break;
	            case KEY_RIGHT://오른쪽 입력
	            	if(curser_positionX != strlen(editNode->text)){//X가 제일오른쪽이 아니면 왼쪽으로 옮겨도 됨. 
	            		curser_positionX++;
					}else{//제일오른쪽이면 
						if(editNode->next !=NULL){
							if(curser_positionY == term_sizeY-3){//커서가 최하단이라면 
								editNode = editNode->next;
								curser_positionX=0;
							}else{//아니라면 
								editNode = editNode->next;
								curser_positionX=0;//x좌표를 0으로 보내고 
								curser_positionY++;//한칸뒤로 보낸다. 	
							} 
							
						} 
					}
	                break;
	            case KEY_LEFT://왼쪽 입력
	            	if(curser_positionX != 0){//커서가 제일 왼쪽이 아니면 
	            		curser_positionX--;//커서를 왼쪽으로 한칸이동해도됨. 
					}else{
						if(editNode->prev != NULL){//커서가 제일 오른쪽이고 앞에 뭐가있다면 
							if(curser_positionY==0){
								editNode = editNode->prev;
							}else{
								editNode = editNode->prev;
								curser_positionX=0;//x좌표를 0으로 보내고 
								curser_positionY--;//한칸 앞으로 보낸다. 
							}
							
						}
					}
	            	break;
	            
	            case KEY_BACKSPACE://백스페이스 입력 
	            case 127: // 백스페이스 ASCII 127
					changetext = 1;
					int size = 0;
					if(editNode->prev !=NULL){
						size = strlen(editNode->prev->text);
					}
					delete(&editor,editNode,curser_positionX, curser_positionY);
					
					if(editNode->prev != NULL){//앞이 NULL이 아니고 
						if(curser_positionX==0){//제일 왼쪽일때 오류구간 
							if(curser_positionY>0){	
								curser_positionY--;
							}
							editNode = editNode->prev;
							curser_positionX = size;
						}
						//오류구간 
						else{
							curser_positionX--;
						}
					}else{
						if(curser_positionX>0){
							curser_positionX--;
						}
					}
					
	                break;
	            case 10: // 엔터 키 입력 
	            case 13: // 엔터 키 (ASCII 코드 13)
	            	if(strlen(editNode->text) == curser_positionX){//화면 맨끝에서 엔터를 치면 
	            		if(editNode->next == NULL){//만약에 뒤에 노드가 없다면 
	            			addNode(&editor,"");
	            			curser_positionX = 0;         			
						}else{//뒤에 노드가 있을때 
							makeMidnode(&editor, editNode, "");				
						}
						if(curser_positionY<term_sizeY-3){ //만약 위치가 터미널위치보다 위면 
							editNode = editNode->next;
							//makeMid노드 수정해야됨. 
							curser_positionY++;
						}else{//터미널위치쪽이면
							editNode = editNode->next;
						}
					}else{//글자 잘라서  
						char* buff = (char*)malloc(strlen(editNode->text) + 1);//buf선언하고 
						strcpy(buff, editNode->text); // 버프에 값 저장하고 
						free(editNode->text); //에딧노드 초기화하고
						editNode->text = (char*)malloc(strlen(buff) + 1);//에딧노드 크기 재배열. 
						strncpy(editNode->text, buff, curser_positionX); //앞부분 잘라서 buff입력.
						editNode->text[curser_positionX] = '\0';
						strcpy(buff, buff + curser_positionX); //뒷부분 잘라서 그대로 저장.
						makeMidnode(&editor, editNode, buff);
						free(buff);
					}
					curser_positionX = strlen(editNode->text);//X위치 바꿈 
				
	                break;
	            case KEY_FIND: // 커맨드+f 입력 
	            case 6: //컨트롤+f 입력 
	            	msg = 1;
					editMode = 2; 
	                break;
	            case KEY_SDC: // 커맨드+s 입력 
	            case 19: //컨트롤+s 입력 
	            	if(fileName[0] != '\0'){//이프문 때려용 
						msg = 2;//지정한 파일이름이 있을때
						fileSave(&editor, fileName);
					}else{
						editMode = 1;
						msg = 3; //저장한 파일이름이 없을때
					}
	            	changetext = 0;
	                break;
	            case KEY_EXIT: // 커맨드+q 입력 
	            case 17: //컨트롤+q 입력 
	            	if(changetext == 0){//저장안하고 파일 변화있을때 ctrl -q 한번 감지 용. 
	            		end = 0;
					}else{
						changetext = 0; //한번더 누를때 종료가능하게 바꿈 
						msg = 4;//
					}
	                break;
	            case KEY_HOME: //홈키 입력 
                	editNode = editor.head;
					curser_positionX=0;
					curser_positionY=0;
                	break;
            	case KEY_END://엔드키입력 
                	editNode = editor.tail;
					curser_positionX=0;
					curser_positionY=0;
                	break; 
	            default:
	                if (32 <= key && key <= 126) {// 문자 입력처리 
	                   editLine(editNode, key, curser_positionX);
	                   curser_positionX++;
	                   changetext=1;
	                }
	                break;
	        }
		}else if(editMode == 1){//파일 이름 저장모드 
			int length = strlen(fileName);//현재 노드의 길이를 받아와서 	
			//밑에는 후처리 코드. 
			switch(key){
				case 10: // 엔터 키 입력 
	            case 13: // 엔터 키 (ASCII 코드 13)
	            	if(fileName[0] == '\0'){ //파일이름이 지정되지않은상태에서 엔터치면. 
	            		msg = 5;
	            	}else{
	            		fileSave(&editor, fileName);
	            		msg = 3;
					}
	            	editMode = 0;
					break;
				case KEY_DC: //백스페이스 입력 
	            case KEY_BACKSPACE://백스페이스 입력 
	            case 127:
	            	if(length > 0){
	            		fileName[length -1] = '\0';           		
					}
					break;	
				default://키입력
					if (32 <= key && key <= 126) {// 문자 입력처리 
						
						fileName = (char*)realloc(fileName, length + 2); //\0도 넣어야되서 두자리추가 
						
						fileName[length] = key;
						fileName[length+1] = '\0';	
					}
					break;
			}
			printw("%s", fileName);//출력구간 	
		}else{ //검색모드
			Node* savedNode;
			int savedCurserY;
			int savedCurserX;
			Node* searchNode;//검색을 수행할 노드.
			//char* searchPoistion;  구버전에 쓰이던 것.
			int length = strlen(searchText);//길이 초기화 
			int fnum;
			if(searchStart == 0){//저장모드 
			
				switch(key){
					case 10: // 엔터 키 입력 
	            	case 13: // 엔터 키
	            		if(length>0){
	            			searchStart = 1;
	            			searchNode = editor.head;
							savedNode = editNode;
							savedCurserY = curser_positionY;
							savedCurserX = curser_positionX;
							searchStart = 1; 
							editNode = searchNode;
							curser_positionY = 0;
							curser_positionX = 0;
						}else{
							editMode = 0;
						}
	            		break;
	            	case KEY_BACKSPACE://백스페이스 입력 
		            case 127:
		            	if(length > 0){
		            		searchText[length -1] = '\0';
							length--;           		
						}
						break;
					case 27: //esc 원래위치로 돌아감. 
	            		msg = 0;
	            		editMode = 0;
	            		searchStart = 0;
	                	break;	
					default://키입력 
						if (32 <= key && key <= 126) {// 문자 입력처리 
							searchText = (char*)realloc(searchText, length + 1); //\0도 넣어야되서 두자리추가 						
							searchText[length] = key;
							searchText[length+1] = '\0';
							
						}
						break;
				}
			}else{//탐색할 텍스트를 찾았다면 여기로 넘어옴. 키보드 입력으로 다음 노드를 찾던지 이전노드를  찾던지, 종료를 하던지. 
				switch(key){
					
					case KEY_RIGHT://오른쪽 입력 
						msg = 6;
						int nextPosition = searchNext(searchNode->text, searchText, curser_positionX);
						if (nextPosition != -1) {
						    curser_positionX = nextPosition+strlen(searchText);
						}else{
							//curser_positionX=0;
							while(searchNode->next != NULL){
								if(searchBoyerMooreForward(searchNode->next->text, searchText, 0) != -1){
									searchNode = searchNode->next;
									editNode = searchNode;
									curser_positionX = nextPosition+strlen(searchText)+1;
									break;
								}else{
									searchNode = searchNode->next;
									editNode = searchNode;
								}
								
							}
							if(searchNode->next == NULL && searchNext(searchNode->text, searchText, 0) == -1){
								msg = 7;
								curser_positionX=strlen(searchNode->text);
								curser_positionY=0;
								
							}
							
						}
					    break;
					case KEY_LEFT://이부분 버그 수정만 하고 제출. 
						msg = 6;
						if(curser_positionX < strlen(searchText) && editNode==editor.head ){
							msg = 7;
							break;
						}
						int previousPosition = searchPrevious(searchNode->text, searchText, curser_positionX);
						if (previousPosition != -1) {
						    curser_positionX = previousPosition;
						}else{
							curser_positionX=strlen(searchNode->text)-1;
							while(searchNode->prev != NULL){
								if(searchPrevious(searchNode->prev->text, searchText, strlen(searchNode->prev->text)) != -1){
									searchNode = searchNode->prev;
									editNode = searchNode;
									curser_positionX = strlen(searchNode->text);
									break;
								}else{
									searchNode = searchNode->prev;
									editNode = searchNode;
								}
							}
							if(searchNode->prev == NULL && searchPrevious(searchNode->text, searchText, strlen(searchNode->text)) == -1){
								msg = 7;
								curser_positionX=strlen(searchNode->text);
								curser_positionY=0; 
							}
							
						}
						break;
					case 10: // 엔터 키 입력 
					case 13: // 엔터 키 입력
						msg = 0;
	            		editMode = 0;
	            		searchStart = 0;
						break;
					case 27:
						//원래 노드로 돌아감. 
						curser_positionY = savedCurserY;
						curser_positionX = savedCurserX;
						editNode = savedNode;
						msg = 0;
	            		editMode = 0;
	            		searchStart = 0;
						break;	
				}
			}
		}
		
        refresh(); // 화면 업데이트
    }

    endwin(); // ncurses 또는 PDCurses 종료
	
	Node* current = editor.head;//마지막 메모리 정리	
	while(current != NULL){//현재가 NULL이 아닐때까지 
		Node* mem = current; //mem 변수에 current를 넣음 
		current = current -> next; // 첫라인부터 지우기 시작함. 
		free(mem->text); // 텍스트부터 메모리 할당해제 
		free(mem); //노드도 할당해제 
	} 
	free(fileName);
    return 0;
}
