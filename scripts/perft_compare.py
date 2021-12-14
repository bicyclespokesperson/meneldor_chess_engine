#!/usr/bin/env python3

import chess
import os
import sys
import time


'''
Given a position
    get legal moves for that position
    for each move
        print(fen)
        return count of legal moves for that position

u64 Perft(int depth)
{
  MOVE move_list[256];
  int n_moves, i;
  u64 nodes = 0;

  if (depth == 0)
    return 1ULL;

  n_moves = GenerateLegalMoves(move_list);
  for (i = 0; i < n_moves; i++) {
    MakeMove(move_list[i]);
    nodes += Perft(depth - 1);
    UndoMove(move_list[i]);
  }
  return nodes;
}
'''


def perft(board, depth, output_stream, print_moves=False):
    nodes = 0

    if depth == 0:
        return 1

    legal_moves = board.legal_moves
    for move in board.legal_moves:
        board.push(move)
        nodes += perft(board, depth - 1, output_stream, print_moves)

        if print_moves:
            print(f'Move {move}', file=output_stream)
        board.pop()


    if print_moves:
        print(f'{board.fen()}: {nodes} moves', file=output_stream)

    return nodes

def compare(fen, depth):

    base_dir = '/Users/jeremysigrist/Desktop/code_projects/chess_engine'

    board = chess.Board(fen)
    expected_output_filename = os.path.join(base_dir, 'expected.txt')
    actual_output_filename = os.path.join(base_dir, 'actual.txt')
    with open(expected_output_filename, mode='w', encoding='utf-8') as outfile:
        result = perft(board, depth, outfile, false)
        print(f'Perft({depth}) = {result}', file=outfile)

    executable_path = os.path.join(base_dir, 'bin/chess_game')
    os.system(f'{executable_path} {depth} "{fen}" > {actual_output_filename}')

    # Current state: Files are identical for starting position at depth 1. 
    #TODO: Compare results at larger depths and print out summary


def main():

    fen_position_1 = 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'
    fen_position_2 = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"
    fen_position_3 = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -"
    fen_position_4 = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"
    pos_4_sub_1 = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1R1K b kq - 1 1"
    fen_position_5 = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"
    fen_position_6 = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 "

    depth = 1
    if len(sys.argv) not in (2, 3):
        print('Usage: perft_compare.py [depth] [fen (opt)]', file=sys.stderr)
        sys.exit(-1)

    if len(sys.argv) >= 2:
        depth = int(sys.argv[1])

    fen = fen_position_1
    if len(sys.argv) == 3:
        fen = sys.argv[2]

    #compare(fen, depth)

    board = chess.Board(fen)
    start_time = time.time()
    print_moves = False
    result = perft(board, depth, sys.stdout, print_moves)
    end_time = time.time()
    print(f'Perft({depth}) = {result}')
    print(f'Took {end_time - start_time} seconds')
    print(f'Nodes/sec: {result / (end_time - start_time)}')

    # Current Status:
    # Generating one extra move for this position (depth 4): "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
    # (Position 2 on chess wiki perft results)
    # Should be 97862


if __name__ == '__main__':
    main()
