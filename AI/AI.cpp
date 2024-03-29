#include "AI.h"

#include <chrono>
#include <future>
#include <iostream>
#include <limits>
#include <thread>
#include <vector>
#include <cstdlib>
#include <random>

#include "../ChessBoard.h"
#include "PieceSqTable.h"

void AI::makeMove(ChessBoard* board, bool isWhite) {
    auto start_time = std::chrono::high_resolution_clock::now();
    Move bestMove = findBestMove(board, isWhite);

    if (!board->movePiece(bestMove.fromX, bestMove.fromY, bestMove.toX, bestMove.toY)) {
        std::cout << "AI move failed, this should not happen" << std::endl;
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Time to find best move " << duration.count() << " milliseconds\n";
}

Move AI::findBestMove(ChessBoard* board, bool isWhite) {
    float bestScore = -10000;
    Move bestMove;

    auto moves = generateMoves(board, isWhite);

    auto evaluateMove = [&](const Move& move) {
        ChessBoard* newBoard = new ChessBoard(*board);
        newBoard->movePiece(move.fromX, move.fromY, move.toX, move.toY);

        float score = minimax(newBoard, maxDepth, -10000, 10000, isWhite, !isWhite);

        delete newBoard; 

        return score;
    };

    std::vector<std::future<float>> futures;

    for (const auto& move : moves) {
        futures.push_back(std::async(std::launch::async, evaluateMove, move));
    }

    for (size_t i = 0; i < moves.size(); ++i) {
        float score = futures[i].get();

        if (score > bestScore) {
            bestScore = score;
            bestMove = moves[i];
        }
    }

    std::cout << "Best score: " << bestScore << std::endl;

    return bestMove;
}


std::vector<Move> AI::generateMoves(ChessBoard* board, bool isWhite) {
    auto availableMoves = std::vector<Move>();
    uint64_t boardPieces = board->getBoard(isWhite);

    while (boardPieces) {
        int index = __builtin_ctzll(boardPieces);
        int x = index % 8;
        int y = index / 8;

        auto piece = board->getPieceTypeAt(x, y);

        auto attacks = board->getValidAttacks(x, y);
        auto moves = board->getValidMoves(x, y);

        while (attacks) {
            int attackIndex = __builtin_ctzll(attacks);
            int newX = attackIndex % 8;
            int newY = attackIndex / 8;

            Move move = {x, y, newX, newY, 0};

            availableMoves.push_back(move);

            attacks &= attacks - 1;
        }

        while (moves) {
            int moveIndex = __builtin_ctzll(moves);
            int newX = moveIndex % 8;
            int newY = moveIndex / 8;

            Move move = {x, y, newX, newY, 0};

            availableMoves.push_back(move);

            moves &= moves - 1;
        }

        boardPieces &= boardPieces - 1;
    }

    return availableMoves;
}

float AI::minimax(ChessBoard* board, int depth, float alpha, float beta, bool maximizingPlayer, bool isWhite) {
    if (depth == 0) {
        return evaluatePosition(board);
    }
    float bestScore = maximizingPlayer ? -10000 : 10000;

    auto moves = generateMoves(board, isWhite);

    for (const auto& move : moves) {
        auto piece = board->getPieceTypeAt(move.toX, move.toY);
        board->movePiece(move.fromX, move.fromY, move.toX, move.toY);

        float score = minimax(board, depth - 1, alpha, beta, !maximizingPlayer, !isWhite);

        board->undoMove(move.fromX, move.fromY, move.toX, move.toY, piece);

        if (maximizingPlayer) {
            bestScore = std::max(bestScore, score);
            alpha = std::max(alpha, bestScore);
        } else {
            bestScore = std::min(bestScore, score);
            beta = std::min(beta, bestScore);
        }

        if (beta <= alpha) {
            break;  // Prune the search tree
        }
    }

    return bestScore;
}

float AI::evaluatePosition(ChessBoard* board) {
    float whiteScore = 0.0f;
    float blackScore = 0.0f;

    auto whitePieces = board->getBoard(true);
    auto blackPieces = board->getBoard(false);

    while (whitePieces) {
        int index = __builtin_ctzll(whitePieces);
        int x = index % 8;
        int y = index / 8;

        auto piece = board->getPieceTypeAt(x, y);

        whiteScore += getValueForPiece(piece);
        whiteScore += piecePositionScore(x, y, piece, true);

        whitePieces &= whitePieces - 1;
    }

    while (blackPieces) {
        int index = __builtin_ctzll(blackPieces);
        int x = index % 8;
        int y = index / 8;

        auto piece = board->getPieceTypeAt(x, y);

        blackScore += getValueForPiece(piece);
        blackScore += piecePositionScore(x, y, piece, false);

        blackPieces &= blackPieces - 1;
    }

    return blackScore - whiteScore;
}

int AI::piecePositionScore(int x, int y, PieceType type, bool isWhite) {
    int i = isWhite ? 0 : 1;
    switch (type) {
        case PAWN:
            return PAWN_TABLE[i][y * 8 + x];
        case KNIGHT:
            return KNIGHT_TABLE[i][y * 8 + x];
        case BISHOP:
            return BISHOP_TABLE[i][y * 8 + x];
        case ROOK:
            return ROOK_TABLE[i][y * 8 + x];
        case QUEEN:
            return QUEEN_TABLE[i][y * 8 + x];
        case KING:
            return KING_TABLE[i][y * 8 + x];
        default:
            return 0;
    }
}

float AI::getValueForPiece(PieceType piece) {
    switch (piece) {
        case PAWN:
            return 30.0f;
        case KNIGHT:
            return 70.0f;
        case BISHOP:
            return 70.0f;
        case ROOK:
            return 10.0f;
        case QUEEN:
            return 200.0f;
        case KING:
            return 1000.0f;
        default:
            return 0.0f;
    }
}