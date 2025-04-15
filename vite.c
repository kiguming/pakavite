#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <curses.h>
#else
#include <ncurses.h>
#endif

typedef struct Node {
    char *text; //�ؽ�Ʈ�κ� 
    struct Node *next; //������带 ����Ŵ 
    struct Node *prev; //������带 ����Ŵ 
} Node;

typedef struct Editor{
    Node *head; //��ü�� ���� �� 
    Node *tail; //��ü�� ���� �� 
    int total_lines; //���� ������ ����
    int top_line; //�ؽ�Ʈ�����Ϳ��� ���� �ֻ���� ù���� 
} Editor;

void initEditor(Editor *editor) { //����ü �ʱ�ȭ 
    editor->head = NULL;
    editor->tail = NULL;
    editor->total_lines = 0;
    editor->top_line = 0;
}

void addNode(Editor *editor, char* text){
	Node *newNode = (Node *)malloc(sizeof(Node)); // �� ��� ���� �Ҵ�
    newNode->text = strdup(text); // �־��� �ؽ�Ʈ�� �����Ͽ� ��忡 ����
    newNode->next = NULL; // �ϴ� �ʱ�ȭ 
    newNode->prev = NULL; // �ϴ� �ʱ�ȭ

    if (editor->head == NULL) {// ���࿡ editor�� ����ִٸ� 
        editor->head = newNode; //�Ӹ����� ����尡 ���� 
        editor->tail = newNode; //�������� ����带 �ְ� 
    } else {
        newNode->prev = editor->tail; // ���� ���� �������ִ� ���� ������� �տ� �ְ� 
        editor->tail->next = newNode; // ���� ������ ������ ����带 �ְ� 
        editor->tail = newNode; //����带 ������ ���� 
    }
    editor->total_lines++; // ��ü ���� �� ����
}


void displayScreen(Editor* editor,Node* editNode, int term_sizeY, int term_sizeX,int curser_positionY){
	int i = 0;
	int nofindNext = 0;
	 
	Node* screenNode = editNode;
	if(screenNode->prev !=NULL){
		for(i=0;i<curser_positionY;i++){
			screenNode = screenNode->prev;//��ũ����尡 Ŀ���� Y�� �ֻ�ܱ��� �̵���Ŵ. 
		}
	} 
	
	
	for(i=0;i<term_sizeY-2;i++){
		if(screenNode->text != NULL && nofindNext == 0  ){//��ũ����忡 �ؽ�Ʈ ���� �ִٸ�
		printw("%.*s\n",term_sizeX-1, screenNode->text);//����ϰ� 
		}else{
			printw("~\n"); //���ٸ� ~�� ����ض�. 
		}
		if(screenNode->next != NULL){//��ũ�� ����� ������ �ִٸ� 
			screenNode = screenNode->next;//������� �̵��ض� 
		}else{
			nofindNext =1;
		} 
		
	}

}


void statusBar(int curser_positionY, int curser_positionX,char* fileName,int linesize,int term_sizeX){
	start_color();  // ������ Ŵ 
    init_pair(1, COLOR_WHITE, COLOR_BLACK);  // ��� ��濡 ���� ����
    init_pair(2, COLOR_BLACK, COLOR_WHITE);  // ���� ��濡 ��� ���� 
	
	int size = 0;
	size += strlen(fileName); // �����̸� ������ 
	size += 28; //��Ÿ����� ������; 
	
	size = term_sizeX - size;//x,y��°� �����̸� ���̿� ���� 
	int i=0; 
	
	attron(COLOR_PAIR(1) | A_REVERSE);
    printw("[%s] - %10d Lines",fileName,linesize);//24��°���� ���� ��
    for(i=0;i<size;i++){
    	printw(" ");//������ ��ŭ �߰� ���� ��� 
	}
    printw("%3d %3d\n",curser_positionY,curser_positionX);
    attroff(COLOR_PAIR(1) | A_REVERSE);
}

void msgBar(int msg,char* searchText){
	switch(msg){
		case 0:
			printw("Help:ctrl-s = Save | ctrl-q = quit | ctrl-F = find");//25��°���� �޽�����
			break;
		case 1:
			printw("search text : %s  + enter / out mode = esc", searchText);//25��°���� �޽�����
			break;
		case 2:
			printw("saved file.");
			break;
		case 3:
			printw("saved mode. if you want to save. FIle name + enter");
			break;	
		case 4:
			printw("if you quit. one more ctrl-q.");//25��°���� �޽�����
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

void editLine(Node* editNode, char key,int curser_positionX){//��������� �����ϴ� ���� 
	
	int length = strlen(editNode->text);//���� ����� ���̸� �޾ƿͼ� 
	editNode->text = (char*)realloc(editNode->text, length + 2); //\0�� �־�ߵǼ� ���ڸ��߰� 
	if(length == curser_positionX){//���� �Ǹ������� �߰��ϴ°� 
		editNode->text[length] = key;
		editNode->text[length+1] = '\0';
	}else{
		memmove(editNode->text + curser_positionX + 1, editNode->text + curser_positionX, length - curser_positionX + 1);
		editNode->text[curser_positionX] = key;
	}

	
}

void delete(Editor* editor, Node* editNode, int curser_positionX,int curser_positionY){
	
	int length = strlen(editNode->text);//���� ����� ������ 
	if(editNode->prev != NULL){//�տ� ��尡 �־�ߵǿ�. 
		if(curser_positionX == 0){//X�� �� �����϶�
			if(editNode->text[0] == '\0'){//�ؽ�Ʈ ������ ���ٸ� 
				if(editNode->next != NULL){//������� �ڿ� ����������
					Node* nextNode = editNode->next; 
					editNode = editNode->prev; //������� ��ĭ����� 
					free(editNode->next); //������� �� ������ 
					editNode->next = nextNode; 
					nextNode->prev = editNode;
				}else{//������ 
					editNode = editNode->prev;
					editor->tail = editNode;
					free(editNode->next);
					editNode->next = NULL;
				}	
			}else{ 
				char* buffer = (char*)malloc(strlen(editNode->text));//���� ���� ���� 
				strcpy(buffer,editNode->text);//���ۿ� �� �ӽ�������
				buffer[strlen(editNode->text)] = '\0';//������� 1������
				if(editNode->next ==NULL){//������� �ڰ� ���ٸ�, 
					editNode = editNode->prev;//������ �̵��ϰ� 
					editor->tail = editNode;//������ ���� ������带 �ְ� 
					free(editNode->next->text);//next�� �����Ŵϱ� ����� 
					free(editNode->next);//next�� �����Ŵϱ� ����� 
					editNode->next = NULL;//���� ������带 ���ְ� 
				}else{//������� �ڿ� ��尡�ִٸ� 
					Node* nextNode =  editNode->next;//�ӽ� �� ��� ���� 
					editNode =  editNode->prev; //���� ��带 �ճ��� ���� 
					nextNode->prev = editNode; //�ճ�带 next����� ���̶�� �����ϰ� 
					editNode->next = nextNode; //���� ��忡 �ؽ�Ʈ ��带 �����ϰ�
					
				}
				
				editNode->text = (char*)realloc(editNode->text,strlen(editNode->text) + length);//ũ�� ����� 
				strcat(editNode->text, buffer);
				
				free(buffer);
			} 
		editor->total_lines--;//�� ó���ϰ� ����ĭ ����. 	 
		}else{//X�� �� ������ �ƴҶ� 
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
    newNode->text = strdup(text); // �־��� �ؽ�Ʈ�� �����Ͽ� ��忡 ����
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

    // editor�� head�� tail ������ ������Ʈ 
    if (prevNode == NULL) {
        editor->head = newNode;
    }
    if (nextNode == NULL) {
        editor->tail = newNode;
    }

    // total_lines ī��Ʈ ����
    editor->total_lines++;
    
}

void fileSave(Editor* editor,char* fileName){
	FILE* file = fopen(fileName, "w");
	Node* sNode = editor->head;
	if(file != NULL){
		while(sNode->next != NULL){ //s����� �ڰ� ���������� 
		char* buffer = (char*)malloc(strlen(sNode->text) + 2);  // +1�ϴϱ� ���ڱ���. \n�� \0�� �־���� 
        strcpy(buffer, sNode->text);//���ۿ� �ؽ�Ʈ ����. 
        strcat(buffer, "\n");
        fputs(buffer, file);
        free(buffer);  // �޸� �� ���� 
        sNode = sNode->next;
		}
		//��������� ó�� 
		char* buffer = (char*)malloc(strlen(sNode->text) + 2);  // +1�ϴϱ� ���ڱ���. \n�� \0�� �־���� 
        strcpy(buffer, sNode->text);
        strcat(buffer, "\n");
        fputs(buffer, file);
        free(buffer);
	}
	fclose(file);
	
	
}

int searchBoyerMooreForward(char *text, char *searchText, int start) {
    int m = strlen(searchText); //�˻��� �ؽ�Ʈ ������ 
    int n = strlen(text); //�ؽ�Ʈ ��ü������. 
    int s = start; //�ؽ�Ʈ���� ������ ��ġ 
    while (s <= (n - m)) { //��������ġ�� �����ؽ�Ʈ���� �������� 
        int j = m - 1;
        while (j >= 0 && searchText[j] == text[s + j]) {
            j--;
        }
        if (j < 0) {//������ ������ ��ġ�ϸ� 
            return s;//ã�� ���� �Ǹ�������ġ�� ���ڸ� ���� 
        } else {
            s += 1;//�װԾƴϸ� ������ġ �̵� 
        }
    }

    return -1; // ��ã���� -1 
}

int searchNext(char *text, char *searchText, int start) {
    int nextPosition = searchBoyerMooreForward(text, searchText, start + 1);
    return nextPosition;
}

int searchPrevious(char *text, char *searchText, int start) { //���̾�� �ݴ����. 
    int m = strlen(searchText);
    int i = 0;
    int j = 0;
    for (i = start - 1; i >= 0; i--) { //������ �ݴ�� ���Ƽ� 
        int match = 1; 
        for (j = 0; j < m; j++) {
            if (text[i + j] != searchText[j]) {
                match = 0; 
                break;
            }
        }
        if (match == 1) {
            return i; //��ġ�Ѵٸ� ���� ���� �������� 
        }
    }
    return -1; //��ã���� -1�� �������� 
}


int main(int argc, char *argv[]) {
	Editor editor;         //������ ����ü ����. 
	initEditor(&editor);   //������ �ʱ�ȭ 
	
	
	
	char* fileName =malloc(1);//�����̸� ���� ����. 
	*fileName ='\0'; //�����̸� �ϴ� ���°ɷ� 
	if(argc == 2){//�Ű������� �ִٸ� 
		fileName = malloc(strlen(argv[1]) + 1); //�Ű����� ��ŭ ������ �ٽ� �ø��� 
		strcpy(fileName, argv[1]);//fileName�� �̸� ���� 
		FILE *file = fopen(argv[1], "r");//���� �б�,���� ���. 
    	if(file == NULL){//�����̾��ٸ� 
    		return 0;//�׳� �ٷβ����� 
		}
    	char buffer[1024];//�� ���� �ӽù��� 
    	//�����ѹ��ؾߵ�. 
    	while(fgets(buffer, sizeof(buffer), file) != NULL){//������ ������ �˻��� 
    		
    		char *newline = strchr(buffer, '\n');
	        if (newline != NULL) {
	            *newline = '\0';
	        }
    		addNode(&editor, buffer);
		}
		
    	fclose(file);
	}
	
	if(editor.head ==NULL){//��忡 �ƹ��͵� ������ 
		addNode(&editor,""); //��� �����ϱ� 
	}
	
	Node* editNode = editor.head;
	int editMode = 0; //������ ��� 0�̸� �⺻, 1�̸� ������ ���ϸ��Է�, 2�̸� �˻���� 
	int term_sizeY = 26; //�ӽ÷� ������ �͹̳� ������ 
	int term_sizeX = 80; //�ӽ÷� ������ �͹̳� ������ 
    int curser_positionY = 0;// Ŀ���� Y�� �� 
    int curser_positionX = 0;// Ŀ���� X�� ��
    int changetext = 0; //�ؽ�Ʈ�� �ٲ������ Ȯ���ϴ� ��
	//�޼����� ����ϰų� ��� �ٲٴ� �� 0�⺻�� 1�˻� 2���� 3���� 
	int end = 1;//ctrl - q�� ���ؼ� ��ȭ�ϴ� ��.
	int msg = 0;
    initscr();            // Ŀ�� �ʱ�ȭ
    raw();                // Raw ��� ����
    keypad(stdscr, TRUE); // Ư�� Ű ��� �����ϵ��� ����
    noecho();             // Ű �Է� �� ���� ��� ��Ȱ��ȭ
    curs_set(1);          // Ŀ�� ���̱�
	getmaxyx(stdscr, term_sizeY, term_sizeX);
	
	char* searchText =malloc(1); 
	*searchText ='\0';
    int searchStart = 0;
    while (end) {
        clear(); // ȭ�� �����
        displayScreen(&editor, editNode, term_sizeY, term_sizeX,curser_positionY);//ȭ�麸�̱� 
		statusBar(curser_positionY,curser_positionX,fileName,editor.total_lines,term_sizeX);
		msgBar(msg, searchText);
        move(curser_positionY, curser_positionX);
		
        int key = getch(); //Ű �Է�
		
		
			
		if(editMode == 0){//�Է¸���϶�
			msg = 0; 
	        // Ű ó��
	        switch (key) {
	        	case KEY_PPAGE: // ������ �� �Է� 
	        	case KEY_UP: //�� �Է�
	        		if(curser_positionY == 0){//Ŀ���� ���� ���� 
						if(editNode->prev !=NULL){//���� ��� ���� �ִٸ� 
	        				editNode = editNode -> prev;//������� ���� �ø� 
						}//Y�� 0�̰� editNode�� Head��� �ƹ� �ϵ� �� �Ͼ 
					}else{
						editNode = editNode -> prev; //��ĭ���� ���� 
						curser_positionY--; //Ŀ�� ��ġ�� �ű��	
					} 
					if(curser_positionX > strlen(editNode->text)){//X�� editNode ũ�⺸�� �����ʿ� �ִٸ� 
							curser_positionX = strlen(editNode->text);//X��ġ�� �ٲ�. 
					}
	                break;
	            case KEY_NPAGE://������ �ٿ��Է� 
	            case KEY_DOWN://�Ʒ� �Է� 
	            	if(editNode ->next !=NULL){//��� �ؿ� �����ִ��� ���� Ȯ�� 
	            		if(curser_positionY == term_sizeY-3){//�͹̳λ�����-2���� �۴ٸ� 
							editNode = editNode->next;//���� �ϴ��̴� ��常 �������� �������.
							 
						}else{
							curser_positionY++; //������ �������� 
							editNode = editNode->next;//�����ġ�� ������ 
						}
						if(curser_positionX > strlen(editNode->text)){//X�� editNode ũ�⺸�� �����ʿ� �ִٸ� 
							curser_positionX = strlen(editNode->text);//X��ġ�� �ٲ�. 
						}
					}
	                break;
	            case KEY_RIGHT://������ �Է�
	            	if(curser_positionX != strlen(editNode->text)){//X�� ���Ͽ������� �ƴϸ� �������� �Űܵ� ��. 
	            		curser_positionX++;
					}else{//���Ͽ������̸� 
						if(editNode->next !=NULL){
							if(curser_positionY == term_sizeY-3){//Ŀ���� ���ϴ��̶�� 
								editNode = editNode->next;
								curser_positionX=0;
							}else{//�ƴ϶�� 
								editNode = editNode->next;
								curser_positionX=0;//x��ǥ�� 0���� ������ 
								curser_positionY++;//��ĭ�ڷ� ������. 	
							} 
							
						} 
					}
	                break;
	            case KEY_LEFT://���� �Է�
	            	if(curser_positionX != 0){//Ŀ���� ���� ������ �ƴϸ� 
	            		curser_positionX--;//Ŀ���� �������� ��ĭ�̵��ص���. 
					}else{
						if(editNode->prev != NULL){//Ŀ���� ���� �������̰� �տ� �����ִٸ� 
							if(curser_positionY==0){
								editNode = editNode->prev;
							}else{
								editNode = editNode->prev;
								curser_positionX=0;//x��ǥ�� 0���� ������ 
								curser_positionY--;//��ĭ ������ ������. 
							}
							
						}
					}
	            	break;
	            
	            case KEY_BACKSPACE://�齺���̽� �Է� 
	            case 127: // �齺���̽� ASCII 127
					changetext = 1;
					int size = 0;
					if(editNode->prev !=NULL){
						size = strlen(editNode->prev->text);
					}
					delete(&editor,editNode,curser_positionX, curser_positionY);
					
					if(editNode->prev != NULL){//���� NULL�� �ƴϰ� 
						if(curser_positionX==0){//���� �����϶� �������� 
							if(curser_positionY>0){	
								curser_positionY--;
							}
							editNode = editNode->prev;
							curser_positionX = size;
						}
						//�������� 
						else{
							curser_positionX--;
						}
					}else{
						if(curser_positionX>0){
							curser_positionX--;
						}
					}
					
	                break;
	            case 10: // ���� Ű �Է� 
	            case 13: // ���� Ű (ASCII �ڵ� 13)
	            	if(strlen(editNode->text) == curser_positionX){//ȭ�� �ǳ����� ���͸� ġ�� 
	            		if(editNode->next == NULL){//���࿡ �ڿ� ��尡 ���ٸ� 
	            			addNode(&editor,"");
	            			curser_positionX = 0;         			
						}else{//�ڿ� ��尡 ������ 
							makeMidnode(&editor, editNode, "");				
						}
						if(curser_positionY<term_sizeY-3){ //���� ��ġ�� �͹̳���ġ���� ���� 
							editNode = editNode->next;
							//makeMid��� �����ؾߵ�. 
							curser_positionY++;
						}else{//�͹̳���ġ���̸�
							editNode = editNode->next;
						}
					}else{//���� �߶�  
						char* buff = (char*)malloc(strlen(editNode->text) + 1);//buf�����ϰ� 
						strcpy(buff, editNode->text); // ������ �� �����ϰ� 
						free(editNode->text); //������� �ʱ�ȭ�ϰ�
						editNode->text = (char*)malloc(strlen(buff) + 1);//������� ũ�� ��迭. 
						strncpy(editNode->text, buff, curser_positionX); //�պκ� �߶� buff�Է�.
						editNode->text[curser_positionX] = '\0';
						strcpy(buff, buff + curser_positionX); //�޺κ� �߶� �״�� ����.
						makeMidnode(&editor, editNode, buff);
						free(buff);
					}
					curser_positionX = strlen(editNode->text);//X��ġ �ٲ� 
				
	                break;
	            case KEY_FIND: // Ŀ�ǵ�+f �Է� 
	            case 6: //��Ʈ��+f �Է� 
	            	msg = 1;
					editMode = 2; 
	                break;
	            case KEY_SDC: // Ŀ�ǵ�+s �Է� 
	            case 19: //��Ʈ��+s �Է� 
	            	if(fileName[0] != '\0'){//������ ������ 
						msg = 2;//������ �����̸��� ������
						fileSave(&editor, fileName);
					}else{
						editMode = 1;
						msg = 3; //������ �����̸��� ������
					}
	            	changetext = 0;
	                break;
	            case KEY_EXIT: // Ŀ�ǵ�+q �Է� 
	            case 17: //��Ʈ��+q �Է� 
	            	if(changetext == 0){//������ϰ� ���� ��ȭ������ ctrl -q �ѹ� ���� ��. 
	            		end = 0;
					}else{
						changetext = 0; //�ѹ��� ������ ���ᰡ���ϰ� �ٲ� 
						msg = 4;//
					}
	                break;
	            case KEY_HOME: //ȨŰ �Է� 
                	editNode = editor.head;
					curser_positionX=0;
					curser_positionY=0;
                	break;
            	case KEY_END://����Ű�Է� 
                	editNode = editor.tail;
					curser_positionX=0;
					curser_positionY=0;
                	break; 
	            default:
	                if (32 <= key && key <= 126) {// ���� �Է�ó�� 
	                   editLine(editNode, key, curser_positionX);
	                   curser_positionX++;
	                   changetext=1;
	                }
	                break;
	        }
		}else if(editMode == 1){//���� �̸� ������ 
			int length = strlen(fileName);//���� ����� ���̸� �޾ƿͼ� 	
			//�ؿ��� ��ó�� �ڵ�. 
			switch(key){
				case 10: // ���� Ű �Է� 
	            case 13: // ���� Ű (ASCII �ڵ� 13)
	            	if(fileName[0] == '\0'){ //�����̸��� ���������������¿��� ����ġ��. 
	            		msg = 5;
	            	}else{
	            		fileSave(&editor, fileName);
	            		msg = 3;
					}
	            	editMode = 0;
					break;
				case KEY_DC: //�齺���̽� �Է� 
	            case KEY_BACKSPACE://�齺���̽� �Է� 
	            case 127:
	            	if(length > 0){
	            		fileName[length -1] = '\0';           		
					}
					break;	
				default://Ű�Է�
					if (32 <= key && key <= 126) {// ���� �Է�ó�� 
						
						fileName = (char*)realloc(fileName, length + 2); //\0�� �־�ߵǼ� ���ڸ��߰� 
						
						fileName[length] = key;
						fileName[length+1] = '\0';	
					}
					break;
			}
			printw("%s", fileName);//��±��� 	
		}else{ //�˻����
			Node* savedNode;
			int savedCurserY;
			int savedCurserX;
			Node* searchNode;//�˻��� ������ ���.
			//char* searchPoistion;  �������� ���̴� ��.
			int length = strlen(searchText);//���� �ʱ�ȭ 
			int fnum;
			if(searchStart == 0){//������ 
			
				switch(key){
					case 10: // ���� Ű �Է� 
	            	case 13: // ���� Ű
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
	            	case KEY_BACKSPACE://�齺���̽� �Է� 
		            case 127:
		            	if(length > 0){
		            		searchText[length -1] = '\0';
							length--;           		
						}
						break;
					case 27: //esc ������ġ�� ���ư�. 
	            		msg = 0;
	            		editMode = 0;
	            		searchStart = 0;
	                	break;	
					default://Ű�Է� 
						if (32 <= key && key <= 126) {// ���� �Է�ó�� 
							searchText = (char*)realloc(searchText, length + 1); //\0�� �־�ߵǼ� ���ڸ��߰� 						
							searchText[length] = key;
							searchText[length+1] = '\0';
							
						}
						break;
				}
			}else{//Ž���� �ؽ�Ʈ�� ã�Ҵٸ� ����� �Ѿ��. Ű���� �Է����� ���� ��带 ã���� ������带  ã����, ���Ḧ �ϴ���. 
				switch(key){
					
					case KEY_RIGHT://������ �Է� 
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
					case KEY_LEFT://�̺κ� ���� ������ �ϰ� ����. 
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
					case 10: // ���� Ű �Է� 
					case 13: // ���� Ű �Է�
						msg = 0;
	            		editMode = 0;
	            		searchStart = 0;
						break;
					case 27:
						//���� ���� ���ư�. 
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
		
        refresh(); // ȭ�� ������Ʈ
    }

    endwin(); // ncurses �Ǵ� PDCurses ����
	
	Node* current = editor.head;//������ �޸� ����	
	while(current != NULL){//���簡 NULL�� �ƴҶ����� 
		Node* mem = current; //mem ������ current�� ���� 
		current = current -> next; // ù���κ��� ����� ������. 
		free(mem->text); // �ؽ�Ʈ���� �޸� �Ҵ����� 
		free(mem); //��嵵 �Ҵ����� 
	} 
	free(fileName);
    return 0;
}
