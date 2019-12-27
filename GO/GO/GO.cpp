﻿#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include "stack.h"

#define B 1 // Two players
#define W 2
#define FLT_MAX_T 999
#define best_play_v2 best_play

int bsize = 0; // board has size bsize*bsize
char* board;
char* mu_board; // store global black/white state
char* mu_tmp_board; // store first time calculated (tmp) global black/white state
char* mu_tmp2_board; // store first second calculated (tmp) global black/white state
char* pure_board; // died (fake living) piece removed from board
char player;
int* piece_index_buf; // store the piece indices that have already been searched
int* qi_index_buf; // store the qi indices that have already been searched
int piece_buf_size; // store the number of pieces after calling calculate_qi()
int qi_buf_size; // store the number of qis after calling calculate_qi()
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
void copy_board(char* r, char* w);
void calculate_qi(int index, char* board);
void piece_remove_buf(char* b);
int validate_move(int index);
void add_record(int index);
void write_game();
void read_game();
void print_qi();
void calculate_mu(char* b);
float count_mu();
int identify_yan(int qi_index, int our_pieces, int our_side, char* board);
void remove_died(char* b);
void print_best_play();
void do_best_play();

int best_play_v1();
int best_play_v2();

// main function
int main(int argc, char* argv[]) {
	if (argc > 1)
		bsize = atoi(argv[1]);
	if (bsize < 7 || bsize > 26)
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
		if (index == -5) {
			print_qi();
			continue;
		}
		if (index == -6) {
			print_best_play();
			continue;
		}
		if (index == -7) {
			do_best_play();
			continue;
		}
		if (validate_move(index) == -1) {
			printf("Invalid input! Please type again.\n");
			continue;
		}
		calculate_mu(board);
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
	float num_B = count_mu();
	printf("\nCurrent MU: Black VS. White = %.1f : %.1f", num_B, (double)bsize * bsize - num_B);
	printf("\n\n");
}

// Init and end
void init() {
	board = (char*)calloc(bsize * bsize, sizeof(char));
	mu_board = (char*)calloc(bsize * bsize, sizeof(char));
	mu_tmp_board = (char*)calloc(bsize * bsize, sizeof(char));
	mu_tmp2_board = (char*)calloc(bsize * bsize, sizeof(char));
	pure_board = (char*)calloc(bsize * bsize, sizeof(char));
	piece_index_buf = (int*)calloc(bsize * bsize, sizeof(int));
	qi_index_buf = (int*)calloc(bsize * bsize, sizeof(int));
	player_B_list = (int*)calloc(2 * bsize * bsize, sizeof(int)); // Ensure no overflow
	player_W_list = (int*)calloc(2 * bsize * bsize, sizeof(int));
	if (board == NULL || mu_board == NULL || mu_tmp_board == NULL || mu_tmp2_board == NULL || pure_board == NULL ||
		piece_index_buf == NULL || qi_index_buf == NULL || player_B_list == NULL || player_W_list == NULL) {
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
	free(mu_board);
	free(mu_tmp_board);
	free(mu_tmp2_board);
	free(pure_board);
	free(piece_index_buf);
	free(qi_index_buf);
	free(player_B_list);
	free(player_W_list);
}

// Print the board
void print_board() {
	for (int i = 0; i < bsize; i++) {
		// print board
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
		// print pure_board
		if (i < 9)
			printf("\t\t%d ", i + 1);
		else
			printf("\t\t%d", i + 1);
		for (int j = 0; j < bsize; j++) {
			if (pure_board[i * bsize + j] == B) {
				printf("○");
			}
			else if (pure_board[i * bsize + j] == W) {
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
		// print mu_board
		if (i < 9)
			printf("\t\t%d ", i + 1);
		else
			printf("\t\t%d", i + 1);
		for (int j = 0; j < bsize; j++) {
			if (mu_board[i * bsize + j] == B) {
				printf("○");
			}
			else if (mu_board[i * bsize + j] == W) {
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
	printf("\t\t  ");
	for (int i = 0; i < bsize; i++) {
		printf("%c ", 65 + i);
	}
	printf("\t\t  ");
	for (int i = 0; i < bsize; i++) {
		printf("%c ", 65 + i);
	}
	printf("\n");
}

// prompt the user, return index of board (or index of command)
int prompt() {
	printf("Type [row][col] or 0[cmd]: ");
	int row_input;
	char col_inputs[128] = { 0 };
	if (!scanf_s("%d%s", &row_input, &col_inputs, 128) || col_inputs[1] != 0) {
		while (getchar() != '\n'); // Invalid input
		printf("Invalid input! Please type again.\n");
		return -1;
	}
	char col_input = col_inputs[0];
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
		if (col_input == 'i') {
			return -5;
		}
		if (col_input == 's') {
			return -6;
		}
		if (col_input == 'd') {
			return -7;
		}
		printf("Invalid input! Please type again.\n");
		return -1;
	}
	else if (row < 0 || row >= bsize || col < 0 || col >= bsize) {
		printf("Invalid input! Please type again.\n");
		return -1;
	}
	else {
		return index;
	}
}

// copy a board from r to w
void copy_board(char* r, char* w) {
	for (int i = 0; i < bsize * bsize; i++) {
		w[i] = r[i];
	}
}

// calculate_qi helper functions
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
// remove the pieces from the b based on piece_index_buf
void piece_remove_buf(char* b) {
	for (int i = 0; i < piece_buf_size; i++) {
		b[piece_index_buf[i]] = 0;
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
void calculate_qi_helper(int index, char* board) {
	piece_add_index(index);
	// Look at right if it has
	if ((index + 1) % bsize != 0) {
		if (board[index + 1] == 0) {
			if (!qi_contain_index(index + 1)) {
				qi_add_index(index + 1);
			}
		}
		else if (board[index] == board[index + 1]) {
			// If this piece has not been searched
			if (!piece_contain_index(index + 1)) {
				calculate_qi_helper(index + 1, board);
			}
		}
	}
	// Look at left if it has
	if (index % bsize != 0) {
		if (board[index - 1] == 0) {
			if (!qi_contain_index(index - 1)) {
				qi_add_index(index - 1);
			}
		}
		else if (board[index] == board[index - 1]) {
			// If this piece has not been searched
			if (!piece_contain_index(index - 1)) {
				calculate_qi_helper(index - 1, board);
			}
		}
	}
	// Look at top if it has
	if (index >= bsize) {
		if (board[index - bsize] == 0) {
			if (!qi_contain_index(index - bsize)) {
				qi_add_index(index - bsize);
			}
		}
		else if (board[index] == board[index - bsize]) {
			// If this piece has not been searched
			if (!piece_contain_index(index - bsize)) {
				calculate_qi_helper(index - bsize, board);
			}
		}
	}
	// Look at bottom if it has
	if (index < bsize * bsize - bsize) {
		if (board[index + bsize] == 0) {
			if (!qi_contain_index(index + bsize)) {
				qi_add_index(index + bsize);
			}
		}
		else if (board[index] == board[index + bsize]) {
			// If this piece has not been searched
			if (!piece_contain_index(index + bsize)) {
				calculate_qi_helper(index + bsize, board);
			}
		}
	}
}
// calculate qi, assuming board[index] is either B or W
void calculate_qi(int index, char* board) {
	piece_buf_size = 0;
	qi_buf_size = 0;
	calculate_qi_helper(index, board);
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
	if (board[index] != 0) {
		return -1;
	}
	char need_remove = 0;
	// Write the new piece down temporarily
	board[index] = player;
	// test right
	if ((index + 1) % bsize != 0) {
		if (board[index] != board[index + 1] && board[index + 1] != 0) {
			calculate_qi(index + 1, board);
			if (qi_buf_size == 0) {
				if (test_jie(index)) {
					board[index] = 0;
					return -1;
				}
				piece_remove_buf(board);
				need_remove = 1;
			}
		}
	}
	// test left
	if (index % bsize != 0) {
		if (board[index] != board[index - 1] && board[index - 1] != 0) {
			calculate_qi(index - 1, board);
			if (qi_buf_size == 0) {
				if (test_jie(index)) {
					board[index] = 0;
					return -1;
				}
				piece_remove_buf(board);
				need_remove = 1;
			}
		}
	}
	// test top
	if (index >= bsize) {
		if (board[index] != board[index - bsize] && board[index - bsize] != 0) {
			calculate_qi(index - bsize, board);
			if (qi_buf_size == 0) {
				if (test_jie(index)) {
					board[index] = 0;
					return -1;
				}
				piece_remove_buf(board);
				need_remove = 1;
			}
		}
	}
	// test bottom
	if (index < bsize * bsize - bsize) {
		if (board[index] != board[index + bsize] && board[index + bsize] != 0) {
			calculate_qi(index + bsize, board);
			if (qi_buf_size == 0) {
				if (test_jie(index)) {
					board[index] = 0;
					return -1;
				}
				piece_remove_buf(board);
				need_remove = 1;
			}
		}
	}
	// invalid suicide move
	calculate_qi(index, board);
	if (qi_buf_size == 0 && need_remove == 0) {
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
	char buf[128] = { 0 };
	printf("Type file name [filename].go: ");
	scanf_s("%s", buf, 128);
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
	char buf[128] = { 0 };
	int player_B_len = 0, player_W_len = 0;
	int* moves;
	printf("Type file name [filename].go: ");
	scanf_s("%s", buf, 128);
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
	for (int i = 0; i < player_B_len + player_W_len; i++) {
		validate_move(moves[i]);
		calculate_mu(board);
		display();
	}
	free(moves);
	fclose(fp);
}

void print_qi() {
	printf("Look up QI at [row][col]: ");
	int row_input;
	char col_inputs[128] = { 0 };
	if (!scanf_s("%d%s", &row_input, &col_inputs, 128) || col_inputs[1] != 0) {
		while (getchar() != '\n'); // Invalid input
		printf("Invalid lookup\n");
		return;
	}
	char col_input = col_inputs[0];
	int row = row_input - 1;
	int col = col_input - 65;
	int index = row * bsize + col;
	if (row < 0 || row >= bsize || col < 0 || col >= bsize || board[index] == 0) {
		printf("Invalid lookup\n");
	}
	else {
		calculate_qi(index, board);
		printf("QI at %d%c: %d\n", 1 + index / bsize, index % bsize + 65, qi_buf_size);
	}

}

void calculate_mu_helper(char* b, char* w, int ext) {
	// Set w as b
	copy_board(b, w);
	// Set empty slot based on 4 directions
	for (int i = 0; i < bsize * bsize; i++) {
		if (w[i] == 0) {
			// Go though mu_board in 4 directions until encounter non-empty piece
			char num_B = 0;
			char num_W = 0;
			// right
			for (int j = i + 1; j % bsize != 0 && j <= i + ext; j++) {
				if (b[j] == B) {
					num_B++;
					break;
				}
				if (b[j] == W) {
					num_W++;
					break;
				}
			}
			// left
			for (int j = i - 1; (j + 1) % bsize != 0 && j >= i - ext; j--) {
				if (b[j] == B) {
					num_B++;
					break;
				}
				if (b[j] == W) {
					num_W++;
					break;
				}
			}
			// top
			for (int j = i - bsize; j >= 0 && j >= i - ext * bsize; j -= bsize) {
				if (b[j] == B) {
					num_B++;
					break;
				}
				if (b[j] == W) {
					num_W++;
					break;
				}
			}
			// bottom
			for (int j = i + bsize; j < bsize * bsize && j <= i + ext * bsize; j += bsize) {
				if (b[j] == B) {
					num_B++;
					break;
				}
				if (b[j] == W) {
					num_W++;
					break;
				}
			}
			if (num_B != 0 && num_W == 0) {
				w[i] = B;
			}
			if (num_W != 0 && num_B == 0) {
				w[i] = W;
			}
		}
	}
}
// Calculate mu and write the result in mu_board
// First version: ignore the existance of fake living pieces
// Second version: has removed fake living pieces
void calculate_mu(char * b) {
	remove_died(b);
	calculate_mu_helper(pure_board, mu_tmp_board, 1);
	calculate_mu_helper(mu_tmp_board, mu_tmp2_board, 1);
	calculate_mu_helper(mu_tmp2_board, mu_board, 2);
}

// Count mu through mu_board, return black's number of mus
// white's number of mus = bsize * bsize - black's number of mus
float count_mu() {
	float num_B = 0;
	for (int i = 0; i < bsize * bsize; i++) {
		if (mu_board[i] == B) {
			num_B += 1;
		}
		if (mu_board[i] == 0) {
			num_B += 0.5;
		}
	}
	return num_B;
}

// Identify whether a qi is a yan or not, our_pieces specifies the number of our pieces needed
// our_side is either B or W
// return 1 if it's a yan
int identify_yan(int qi_index, int our_pieces, int our_side, char* board) {
	int num_our_piece = 0;
	// right
	if ((qi_index + 1) % bsize != 0) {
		// enemy
		if (board[qi_index + 1] != 0 && board[qi_index + 1] != our_side) {
			return 0;
		}
		if (board[qi_index + 1] == our_side) {
			num_our_piece++;
		}
	}
	// left
	if (qi_index % bsize != 0) {
		// enemy
		if (board[qi_index - 1] != 0 && board[qi_index - 1] != our_side) {
			return 0;
		}
		if (board[qi_index - 1] == our_side) {
			num_our_piece++;
		}
	}
	// top
	if (qi_index >= bsize) {
		// enemy
		if (board[qi_index - bsize] != 0 && board[qi_index - bsize] != our_side) {
			return 0;
		}
		if (board[qi_index - bsize] == our_side) {
			num_our_piece++;
		}
	}
	// bottom
	if (qi_index + bsize < bsize * bsize) {
		// enemy
		if (board[qi_index + bsize] != 0 && board[qi_index + bsize] != our_side) {
			return 0;
		}
		if (board[qi_index + bsize] == our_side) {
			num_our_piece++;
		}
	}
	if (num_our_piece >= our_pieces) {
		return 1;
	}
	return 0;
}

// Remove died pieces from board and write the result to pure_board
void remove_died(char* b) {
	// Copy
	copy_board(b, pure_board);
	for (int i = 0; i < bsize * bsize; i++) {
		if (pure_board[i] == 0) {
			continue;
		}
		calculate_qi(i, b);
		// died piece
		if (qi_buf_size == 1) {
			piece_remove_buf(pure_board);
			continue;
		}
		// must contain 1 strict yans and 1 less strict yan(i.e 2 qis are yans)
		else if (qi_buf_size == 2) {
			int num_yan_s = 0;
			int num_yan_ls = 0;
			for (int j = 0; j < qi_buf_size; j++) {
				int qi_index = qi_index_buf[j];
				if (identify_yan(qi_index, 2, pure_board[i], b)) {
					num_yan_s++;
				}
				if (identify_yan(qi_index, 1, pure_board[i], b)) {
					num_yan_ls++;
				}
			}
			if (num_yan_s < 1 || num_yan_ls < 2) {
				piece_remove_buf(pure_board);
				continue;
			}
		}
		// must contain 2 at least less strict yans (i.e 2 qis are yans)
		else if (qi_buf_size <= 5) {
			int num_yan = 0;
			for (int j = 0; j < qi_buf_size; j++) {
				int qi_index = qi_index_buf[j];
				if (identify_yan(qi_index, 1, pure_board[i], b)) {
					num_yan++;
				}
			}
			if (num_yan < 2) {
				piece_remove_buf(pure_board);
				continue;
			}
		}
	}
}

// Print the result of best_play()
void print_best_play() {
	int index = best_play();
	if (index == -1) {
		printf("Suggestion: STOP\n");
	}
	else {
		printf("Suggestion: %d%c\n", index / bsize + 1, index % bsize + 65);
	}
}

// Do the result of best_play()
void do_best_play() {
	int index = best_play();
	if (index == -1) {
		printf("Suggestion: STOP\n");
		return;
	}
	if (validate_move(index) == -1){
		printf("ERROR: index [%d%c] returned by best_play() is invalid\n", index / bsize + 1, index % bsize + 65);
		exit(-3);
		return;
	}
	printf("Suggestion: %d%c\n", index / bsize + 1, index % bsize + 65);
	calculate_mu(board);
	display();
}

// Return the score of that index. Here, the score is the mu - pre_score
// If the score is -FLT_MAX, the index is not valid
float best_play_v1_helper(int index, float pre_score) {
	if (board[index] != 0)
		return -FLT_MAX_T;
	char* test_board = (char*)calloc(bsize * bsize, sizeof(char));
	if (test_board == NULL) {
		printf("Memory Error in best_play_v1_helper()\n");
		exit(-2);
	}
	copy_board(board, test_board);
	char need_remove = 0;
	// Write the new piece down temporarily
	test_board[index] = player;
	// test right
	if ((index + 1) % bsize != 0) {
		if (test_board[index] != test_board[index + 1] && test_board[index + 1] != 0) {
			calculate_qi(index + 1, test_board);
			if (qi_buf_size == 0) {
				// test jie
				if (index == eaten_record && piece_buf_size == 1) {
					free(test_board);
					return -FLT_MAX_T;
				}
				piece_remove_buf(test_board);
				need_remove = 1;
			}
		}
	}
	// test left
	if (index % bsize != 0) {
		if (test_board[index] != test_board[index - 1] && test_board[index - 1] != 0) {
			calculate_qi(index - 1, test_board);
			if (qi_buf_size == 0) {
				if (index == eaten_record && piece_buf_size == 1) {
					free(test_board);
					return -FLT_MAX_T;
				}
				piece_remove_buf(test_board);
				need_remove = 1;
			}
		}
	}
	// test top
	if (index >= bsize) {
		if (test_board[index] != test_board[index - bsize] && test_board[index - bsize] != 0) {
			calculate_qi(index - bsize, test_board);
			if (qi_buf_size == 0) {
				if (index == eaten_record && piece_buf_size == 1) {
					free(test_board);
					return -FLT_MAX_T;
				}
				piece_remove_buf(test_board);
				need_remove = 1;
			}
		}
	}
	// test bottom
	if (index < bsize * bsize - bsize) {
		if (test_board[index] != test_board[index + bsize] && test_board[index + bsize] != 0) {
			calculate_qi(index + bsize, test_board);
			if (qi_buf_size == 0) {
				if (index == eaten_record && piece_buf_size == 1) {
					free(test_board);
					return -FLT_MAX_T;
				}
				piece_remove_buf(test_board);
				need_remove = 1;
			}
		}
	}
	// invalid suicide move
	calculate_qi(index, test_board);
	if (qi_buf_size == 0 && need_remove == 0) {
		free(test_board);
		return -FLT_MAX_T;
	}
	calculate_mu(test_board);
	free(test_board);
	float num_B = count_mu();
	if (player == B)
		return num_B - pre_score;
	else
		return bsize * bsize - num_B - pre_score;
}

// Play the best hand
// First version: this version only test the possibilities of the current hand of this player
int best_play_v1() {
	float max_score = 0.5;
	int* index_buf = (int*)calloc(bsize * bsize, sizeof(int));
	if (index_buf == NULL) {
		printf("Memory Error in best_play_v1()\n");
		exit(-2);
	}
	int buf_size = 0;
	calculate_mu(board);
	float pre_score;
	float num_B = count_mu();
	if (player == B)
		pre_score = num_B;
	else
		pre_score = bsize * bsize - num_B;
	for (int i = 0; i < bsize * bsize; i++) {
		float score = best_play_v1_helper(i, pre_score);
		if (score > max_score) {
			max_score = score;
			buf_size = 1;
			index_buf[0] = i;
		}
		else if (score == max_score) {
			index_buf[buf_size] = i;
			buf_size++;
		}
	}
	if (buf_size == 0) {
		free(index_buf);
		return -1;
	}
	else {
		int index = index_buf[rand() % buf_size];
		free(index_buf);
		return index;
	}
}

// Fields used for best_play_v2
struct p {
	int index;
	float score;
};

int cmp(const void* a, const void* b) {
	return ((p*)a)->score < ((p*)b)->score ? 1 : -1; // decreasing by score
}
char* test_board_v2;
p* p_buf;
stack * removed_pieces;
int eaten_record_v2;
int hand_record_v2;
int hand_v2;
char player_v2;

// This function validate move on test_board_v2
// This function also stores the change to cooperate with best_play_v2_restore()
// Return -1 if failed to validate the move (then no change on the board)
int best_play_v2_validate(int index) {
	if (test_board_v2[index] != 0) {
		return -1;
	}
	// Write the new piece down temporarily
	test_board_v2[index] = player_v2;
	// Buffer store the recovery information. Notice removed_p[0] stores the length of removed pieces
	int* removed_p = (int*)calloc(bsize * bsize + 5, sizeof(int));
	// Store other information
	removed_p[1] = player_v2;
	removed_p[2] = hand_v2;
	removed_p[3] = eaten_record_v2;
	removed_p[4] = hand_record_v2;
	// test right
	if ((index + 1) % bsize != 0) {
		if (test_board_v2[index] != test_board_v2[index + 1] && test_board_v2[index + 1] != 0) {
			calculate_qi(index + 1, test_board_v2);
			if (qi_buf_size == 0) {
				if (index == eaten_record_v2 && piece_buf_size == 1) {
					test_board_v2[index] = 0;
					free(removed_p);
					return -1;
				}
				if (piece_buf_size == 1) {
					eaten_record_v2 = piece_index_buf[0];
					hand_record_v2 = hand_v2 + 1;
				}
				piece_remove_buf(test_board_v2);
				for (int i = 0; i < piece_buf_size; i++) {
					removed_p[++removed_p[0] + 4] = piece_index_buf[i];
				}
			}
		}
	}
	// test left
	if (index % bsize != 0) {
		if (test_board_v2[index] != test_board_v2[index - 1] && test_board_v2[index - 1] != 0) {
			calculate_qi(index - 1, test_board_v2);
			if (qi_buf_size == 0) {
				if (index == eaten_record_v2 && piece_buf_size == 1) {
					test_board_v2[index] = 0;
					free(removed_p);
					return -1;
				}
				if (piece_buf_size == 1) {
					eaten_record_v2 = piece_index_buf[0];
					hand_record_v2 = hand_v2 + 1;
				}
				piece_remove_buf(test_board_v2);
				for (int i = 0; i < piece_buf_size; i++) {
					removed_p[++removed_p[0] + 4] = piece_index_buf[i];
				}
			}
		}
	}
	// test top
	if (index >= bsize) {
		if (test_board_v2[index] != test_board_v2[index - bsize] && test_board_v2[index - bsize] != 0) {
			calculate_qi(index - bsize, test_board_v2);
			if (qi_buf_size == 0) {
				if (index == eaten_record_v2 && piece_buf_size == 1) {
					test_board_v2[index] = 0;
					free(removed_p);
					return -1;
				}
				if (piece_buf_size == 1) {
					eaten_record_v2 = piece_index_buf[0];
					hand_record_v2 = hand_v2 + 1;
				}
				piece_remove_buf(test_board_v2);
				for (int i = 0; i < piece_buf_size; i++) {
					removed_p[++removed_p[0] + 4] = piece_index_buf[i];
				}
			}
		}
	}
	// test bottom
	if (index < bsize * bsize - bsize) {
		if (test_board_v2[index] != test_board_v2[index + bsize] && test_board_v2[index + bsize] != 0) {
			calculate_qi(index + bsize, test_board_v2);
			if (qi_buf_size == 0) {
				if (index == eaten_record_v2 && piece_buf_size == 1) {
					test_board_v2[index] = 0;
					free(removed_p);
					return -1;
				}
				if (piece_buf_size == 1) {
					eaten_record_v2 = piece_index_buf[0];
					hand_record_v2 = hand_v2 + 1;
				}
				piece_remove_buf(test_board_v2);
				for (int i = 0; i < piece_buf_size; i++) {
					removed_p[++removed_p[0] + 4] = piece_index_buf[i];
				}
			}
		}
	}
	// invalid suicide move
	calculate_qi(index, test_board_v2);
	if (qi_buf_size == 0 && removed_p[0] == 0) {
		test_board_v2[index] = 0;
		free(removed_p);
		return -1;
	}
	
	if (player_v2 == B) {
		player_v2 = W;
	}
	else {
		player_v2 = B;
	}
	hand_v2++;
	// erase the record next hand
	if (hand_v2 == hand_record_v2 + 1) {
		eaten_record_v2 = -1;
		hand_record_v2 = 0;
	}
	int * removed_p_2 = (int*)realloc(removed_p, (removed_p[0] + 5) * sizeof(int));
	if (removed_p_2 == NULL) {
		printf("Memory Error in best_play_v2_validate()\n");
		exit(-2);
	}
	push(removed_pieces, (int)removed_p_2);
	return 0;
}

// This function help restore the board (i.e. remove test_board_v2[index] and add back removed pieces)
void best_play_v2_restore(int index) {
	char me = test_board_v2[index];
	char opponent;
	test_board_v2[index] = 0;
	if (me == B) {
		opponent = W;
	}
	else {
		opponent = B;
	}
	int* removed_p = (int *)pull(removed_pieces);
	int i;
	for (i = 0; i < removed_p[0]; i++) { //moved_p[0] stores the length
		int idx = removed_p[i + 5];
		test_board_v2[idx] = opponent;
	}
	player_v2 = removed_p[1];
	hand_v2 = removed_p[2];
	eaten_record_v2 = removed_p[3];
	hand_record_v2 = removed_p[4];
	free(removed_p); 
}


void best_play_v2_helper() {
	calculate_mu(test_board_v2);
	float pre_score;
	float num_B = count_mu();
	if (player == B)
		pre_score = num_B;
	else
		pre_score = bsize * bsize - num_B; // Current mu of this player
	
	for (int i = 0; i < bsize * bsize; i++) {
		// simulate the hand of this player
		if (best_play_v2_validate(i) == -1) {
			p_buf[i] = {i, -FLT_MAX_T};
			continue;
		}
		//float average_post_score = 0;
		//int count_valid = 0;
		float min_score = FLT_MAX_T;
		for (int j = 0; j < bsize * bsize; j++) {
			// simulate the hand of the opponent
			if (best_play_v2_validate(j) == -1) {
				continue;
			}
			calculate_mu(test_board_v2);
			float post_score;
			num_B = count_mu();
			if (player == B)
				post_score = num_B;
			else
				post_score = bsize * bsize - num_B; // Current mu of this player again
			//average_post_score = (average_post_score * count_valid + post_score) / (count_valid + 1);
			//count_valid++; // calculate average post_score
			if (post_score < min_score) {
				min_score = post_score;
			}
			best_play_v2_restore(j);
		}
		best_play_v2_restore(i);
		//p_buf[i] = {i, average_post_score - pre_score};
		p_buf[i] = { i, min_score - pre_score };
	}
	qsort(p_buf, bsize * bsize, sizeof(p), cmp);
}

// Play the best hand
// Second version: this verstion tests the current hand of this player and the next hand of the opponent
int best_play_v2() {
	test_board_v2 = (char*)calloc(bsize * bsize, sizeof(char)); // test_board used to test all possibilities
	p_buf = (p*)calloc(bsize * bsize, sizeof(p)); // Store all possibilities
	removed_pieces = stack_init(bsize * bsize); // Store recovery information
	if (test_board_v2 == NULL || p_buf == NULL || removed_pieces == NULL) {
		printf("Memory Error in best_play_v2()\n");
		exit(-2);
	}
	copy_board(board, test_board_v2);
	eaten_record_v2 = eaten_record;
	hand_record_v2 = hand_record;
	hand_v2 = hand; 
	player_v2 = player; // Copy and paste the state of current game
	best_play_v2_helper();

	free(test_board_v2);
	p first = p_buf[0];
	for (int i = 0; i < bsize * bsize; i++) {
		printf("(%d%c,%.1f) ", p_buf[i].index/bsize + 1,p_buf[i].index%bsize+65,p_buf[i].score);
	}
	free(p_buf);
	stack_end(removed_pieces);
	// The first case happens at the end of the game (2 * bsize is the buffer size to let the game play)
	// The second case happens when this hand cause opponent go stop
	if (first.score > -2 * bsize && first.score < FLT_MAX_T - bsize * bsize) {
		return first.index;
	}
	else {
		return -1;
	}
}

