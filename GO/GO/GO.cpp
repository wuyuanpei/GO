#include <stdio.h>
#include <stdlib.h>
#include <memory>

#define B 1 // Two players
#define W 2

int bsize = 0; // board has size bsize*bsize
char* board;
char player;
int* piece_index_buf; // store the piece indices that have already been searched
int* qi_index_buf; // store the qi indices that have already been searched
int* player_B_list; // store both players' records
int* player_W_list;
int player_B_list_size;
int player_W_list_size;
int hand;
int eaten_record; // record the last eaten piece (for handling jie)
int hand_record; // record the hand of last eaten piece (for handling jie)

void init();
void end();
void print_board();
void display();
int prompt();
void switch_player();
int calculate_qi(int index);
int validate_move(int index);
void add_record(int index);
void write_game();
void read_game();

// main function
int main(int argc, char* argv[]) {
	if (argc > 1)
		bsize = atoi(argv[1]);
	if (bsize == 0)
		bsize = 9; // default board size
	init();
	print_board();
	while (1) {
		int index = prompt();
		if (index == -1)
			continue;
		if (index == -2) {
			end();
			exit(-1);
		}
		if (index == -3) {
			write_game();
			continue;
		}
		if (index == -4) {
			read_game();
			continue;
		}
		if (validate_move(index) == -1)
			continue;
		display();
	}
}

// Display each step
void display() {
	print_board();
	printf("HAND: %d\n", hand);
	printf("BLACK record: ");
	for (int i = 0; i < player_B_list_size; i++)
		printf("%d%c ", player_B_list[i] / bsize + 1, player_B_list[i] % bsize + 65);
	printf("\nWHITE record: ");
	for (int i = 0; i < player_W_list_size; i++)
		printf("%d%c ", player_W_list[i] / bsize + 1, player_W_list[i] % bsize + 65);
	printf("\n\n");
}

// Init and end
void init() {
	board = (char*)calloc(bsize * bsize, sizeof(char));
	piece_index_buf = (int*)calloc(bsize * bsize, sizeof(int));
	qi_index_buf = (int*)calloc(bsize * bsize, sizeof(int));
	player_B_list = (int*)calloc(bsize * bsize, sizeof(int));
	player_W_list = (int*)calloc(bsize * bsize, sizeof(int));
	if (board == NULL || piece_index_buf == NULL || qi_index_buf == NULL ||
		player_B_list == NULL || player_W_list == NULL) {
		printf("Memory Error in init()\n");
		exit(-2);
	}
	player = B;
	player_B_list_size = 0;
	player_W_list_size = 0;
	eaten_record = -1;
	hand = 0;
}

void end() {
	free(board);
	free(piece_index_buf);
	free(qi_index_buf);
	free(player_B_list);
	free(player_W_list);
}

// Print the board
void print_board() {
	for (int i = 0; i < bsize; i++) {
		if (i < 9)
			printf("%d ", i + 1);
		else
			printf("%d", i + 1);
		for (int j = 0; j < bsize; j++) {
			if (board[i * bsize + j] == B) {
				printf("○");
			}
			else if (board[i * bsize + j] == W) {
				printf("●");
			}
			else if (i == 0 && j == 0) {
				printf("┌ ");
			}
			else if (i == 0 && j == bsize - 1) {
				printf("┐ ");
			}
			else if (i == bsize - 1 && j == bsize - 1) {
				printf("┘ ");
			}
			else if (i == bsize - 1 && j == 0) {
				printf("└ ");
			}
			else if (i == 0) {
				printf("┬ ");
			}
			else if (i == bsize - 1) {
				printf("┴ ");
			}
			else if (j == bsize - 1) {
				printf("┤ ");
			}
			else if (j == 0) {
				printf("├ ");
			}
			else {
				printf("┼ ");
			}
		}
		printf("\n");
	}
	printf("  ");
	for (int i = 0; i < bsize; i++) {
		printf("%c ", 65 + i);
	}
	printf("\n");
}

// prompt the user, return index of board (or index of command)
int prompt() {
	printf("Type [row][col] or 0[cmd]: ");
	int row_input;
	char col_input;
	scanf_s("%d%c", &row_input, &col_input, 1);
	int row = row_input - 1;
	int col = col_input - 65;
	int index = row * bsize + col;
	if (row_input == 0) {
		if (col_input == 'q') {
			return -2;
		}
		if (col_input == 'w') {
			return -3;
		}
		if (col_input == 'r') {
			return -4;
		}
		return -1;
	}
	else if (row < 0 || row >= bsize || col < 0 || col >= bsize || board[index] != 0) {
		return -1;
	}
	else {
		return index;
	}
}

// calculate_qi helper functions
int piece_buf_size;
int qi_buf_size;
int piece_contain_index(int index) {
	for (int i = 0; i < piece_buf_size; i++) {
		if (piece_index_buf[i] == index)
			return 1;
	}
	return 0;
}
void piece_add_index(int index) {
	piece_index_buf[piece_buf_size] = index;
	piece_buf_size++;
}
// remove the pieces from the board based on piece_buf
void piece_remove_buf() {
	for (int i = 0; i < piece_buf_size; i++) {
		board[piece_index_buf[i]] = 0;
	}
}
int qi_contain_index(int index) {
	for (int i = 0; i < qi_buf_size; i++) {
		if (qi_index_buf[i] == index)
			return 1;
	}
	return 0;
}
void qi_add_index(int index) {
	qi_index_buf[qi_buf_size] = index;
	qi_buf_size++;
}
// recursively calculate qi
int calculate_qi_helper(int index) {
	int num_qi = 0;
	piece_add_index(index);
	// Look at right if it has
	if ((index + 1) % bsize != 0) {
		if (board[index + 1] == 0) {
			if (!qi_contain_index(index + 1)) {
				num_qi++;
				qi_add_index(index + 1);
			}
		}
		else if (board[index] == board[index + 1]) {
			// If this piece has not been searched
			if (!piece_contain_index(index + 1)) {
				num_qi += calculate_qi_helper(index + 1);
			}
		}
	}
	// Look at left if it has
	if (index % bsize != 0) {
		if (board[index - 1] == 0) {
			if (!qi_contain_index(index - 1)) {
				num_qi++;
				qi_add_index(index - 1);
			}
		}
		else if (board[index] == board[index - 1]) {
			// If this piece has not been searched
			if (!piece_contain_index(index - 1)) {
				num_qi += calculate_qi_helper(index - 1);
			}
		}
	}
	// Look at top if it has
	if (index >= bsize) {
		if (board[index - bsize] == 0) {
			if (!qi_contain_index(index - bsize)) {
				num_qi++;
				qi_add_index(index - bsize);
			}
		}
		else if (board[index] == board[index - bsize]) {
			// If this piece has not been searched
			if (!piece_contain_index(index - bsize)) {
				num_qi += calculate_qi_helper(index - bsize);
			}
		}
	}
	// Look at bottom if it has
	if (index < bsize * bsize - bsize) {
		if (board[index + bsize] == 0) {
			if (!qi_contain_index(index + bsize)) {
				num_qi++;
				qi_add_index(index + bsize);
			}
		}
		else if (board[index] == board[index + bsize]) {
			// If this piece has not been searched
			if (!piece_contain_index(index + bsize)) {
				num_qi += calculate_qi_helper(index + bsize);
			}
		}
	}
	return num_qi;
}
// calculate qi, assuming board[index] is either B or W
int calculate_qi(int index) {
	piece_buf_size = 0;
	qi_buf_size = 0;
	return calculate_qi_helper(index);
}


// add player's record to the list
void add_record(int index) {
	if (player == B) {
		player_B_list[player_B_list_size] = index;
		player_B_list_size++;
	}
	if (player == W) {
		player_W_list[player_W_list_size] = index;
		player_W_list_size++;
	}
}

void switch_player() {
	if (player == B)
		player = W;
	else
		player = B;
}


// return 1 if this move is invalid becuase of jie
int test_jie(int index) {
	if (index == eaten_record && piece_buf_size == 1) {
		return 1;
	}
	if (piece_buf_size == 1) {
		eaten_record = piece_index_buf[0];
		hand_record = hand + 1;
	}
	return 0;
}

// validate the move to index (remove died pieces and write the new piece to board)
// return -1 if not able to validate it
int validate_move(int index) {
	char need_remove = 0;
	// Write the new piece down temporarily
	board[index] = player;
	// test right
	if ((index + 1) % bsize != 0) {
		if (board[index] != board[index + 1] && board[index + 1] != 0) {
			if (calculate_qi(index + 1) == 0) {
				if (test_jie(index)) {
					board[index] = 0;
					return -1;
				}
				piece_remove_buf();
				need_remove = 1;
			}
		}
	}
	// test left
	if (index % bsize != 0) {
		if (board[index] != board[index - 1] && board[index - 1] != 0) {
			if (calculate_qi(index - 1) == 0) {
				if (test_jie(index)) {
					board[index] = 0;
					return -1;
				}
				piece_remove_buf();
				need_remove = 1;
			}
		}
	}
	// test top
	if (index >= bsize) {
		if (board[index] != board[index - bsize] && board[index - bsize] != 0) {
			if (calculate_qi(index - bsize) == 0) {
				if (test_jie(index)) {
					board[index] = 0;
					return -1;
				}
				piece_remove_buf();
				need_remove = 1;
			}
		}
	}
	// test bottom
	if (index < bsize * bsize - bsize) {
		if (board[index] != board[index + bsize] && board[index + bsize] != 0) {
			if (calculate_qi(index + bsize) == 0) {
				if (test_jie(index)) {
					board[index] = 0;
					return -1;
				}
				piece_remove_buf();
				need_remove = 1;
			}
		}
	}
	// invalid suicide move
	if (calculate_qi(index) == 0 && need_remove == 0) {
		board[index] = 0;
		return -1;
	}
	add_record(index);
	switch_player();
	hand++;
	// erase the record next hand
	if (hand == hand_record + 1) {
		eaten_record = -1;
		hand_record = 0;
	}
	return 0;
}

void write_game() {
	char buf[100] = { 0 };
	printf("Type file name [filename].go: ");
	scanf_s("%s", buf, 100);
	FILE* fp;
	fopen_s(&fp, buf, "wb");
	if (fp == NULL) {
		printf("Fail to create/open the file %s\n", buf);
		return;
	}
	fwrite(&bsize, 1, 4, fp);
	fwrite(&player_B_list_size, 1, 4, fp);
	fwrite(&player_W_list_size, 1, 4, fp);
	for (int i = 0; i < player_B_list_size; i++) {
		fwrite(&player_B_list[i], 1, 4, fp);
	}
	for (int i = 0; i < player_W_list_size; i++) {
		fwrite(&player_W_list[i], 1, 4, fp);
	}
	fclose(fp);
	printf("Game stored in the file %s\n", buf);
}

void read_game() {
	char buf[100] = { 0 };
	int player_B_len = 0, player_W_len = 0;
	int* moves;
	printf("Type file name [filename].go: ");
	scanf_s("%s", buf, 100);
	FILE* fp;
	fopen_s(&fp, buf, "rb");
	if (fp == NULL) {
		printf("Fail to open the file %s\n", buf);
		return;
	}
	end();
	fread(&bsize, 1, 4, fp);
	fread(&player_B_len, 1, 4, fp);
	fread(&player_W_len, 1, 4, fp);
	moves = (int*)calloc(player_B_len + player_W_len, sizeof(int));
	if (moves == NULL) {
		printf("Memory Error in read_game()\n");
		exit(-2);
	}
	init();
	print_board();
	for (int i = 0; i < player_B_len; i++) {
		fread(moves + 2 * i, 1, 4, fp);
	}
	for (int i = 0; i < player_W_len; i++) {
		fread(moves + 2 * i + 1, 1, 4, fp);
	}
	for(int i = 0; i< player_B_len + player_W_len; i++) {
		validate_move(moves[i]);
		display();
	}
	free(moves);
	fclose(fp);
}