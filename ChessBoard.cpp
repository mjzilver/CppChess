#include "ChessBoard.h"

#include <cmath>
#include <cstdint>

// board functions
void ChessBoard::resetBoard() {
    emptyBoard();

    // set pawns
    for (int i = 0; i < 8; ++i) {
        setPiece(i, 1, PAWN, false);  
        setPiece(i, 6, PAWN, true);   
    }

    // set rooks
    setPiece(0, 0, ROOK, false);
    setPiece(7, 0, ROOK, false);
    setPiece(0, 7, ROOK, true);
    setPiece(7, 7, ROOK, true);

    // set knights
    setPiece(1, 0, KNIGHT, false);
    setPiece(6, 0, KNIGHT, false);
    setPiece(1, 7, KNIGHT, true);
    setPiece(6, 7, KNIGHT, true);

    // set bishops
    setPiece(2, 0, BISHOP, false);
    setPiece(5, 0, BISHOP, false);
    setPiece(2, 7, BISHOP, true);
    setPiece(5, 7, BISHOP, true);

    // set queens
    setPiece(3, 0, QUEEN, false);
    setPiece(3, 7, QUEEN, true);

    // set kings
    setPiece(4, 0, KING, false);
    setPiece(4, 7, KING, true);

    _gameOver = false;
}

void ChessBoard::emptyBoard() {
    whitePieces = 0;
    blackPieces = 0;
    pieces[PAWN] = 0;
    pieces[ROOK] = 0;
    pieces[KNIGHT] = 0;
    pieces[BISHOP] = 0;
    pieces[QUEEN] = 0;
    pieces[KING] = 0;
}

char ChessBoard::getPieceSymbol(int x, int y) const {
    if (isPieceAt(x, y, PAWN)) return 'P';
    if (isPieceAt(x, y, ROOK)) return 'R';
    if (isPieceAt(x, y, KNIGHT)) return 'N';
    if (isPieceAt(x, y, BISHOP)) return 'B';
    if (isPieceAt(x, y, QUEEN)) return 'Q';
    if (isPieceAt(x, y, KING)) return 'K';
    return ' ';
}

// piece functions
void ChessBoard::setPiece(int x, int y, PieceType pieceType, bool isWhite) {
    uint64_t piece = 1ULL << (x + y * 8);
    if (isWhite) {
        whitePieces |= piece;
    } else {
        blackPieces |= piece;
    }
    pieces[pieceType] |= piece;
}

void ChessBoard::removePiece(int x, int y, PieceType pieceType, bool isWhite) {
    uint64_t piece = 1ULL << (x + y * 8);
    if (isWhite) {
        whitePieces &= ~piece;
    } else {
        blackPieces &= ~piece;
    }
    pieces[pieceType] &= ~piece;
}

bool ChessBoard::isPieceAt(int x, int y) const {
    uint64_t piece = 1ULL << (x + y * 8);
    return (whitePieces & piece) || (blackPieces & piece);
}

bool ChessBoard::isPieceAt(int x, int y, bool isWhite) const {
    uint64_t piece = 1ULL << (x + y * 8);
    return isWhite ? (whitePieces & piece) : (blackPieces & piece);
}

bool ChessBoard::isPieceAt(int x, int y, PieceType pieceType) const {
    uint64_t piece = 1ULL << (x + y * 8);
    return pieces[pieceType] & piece;
}

bool ChessBoard::getPieceColor(int x, int y) const {
    uint64_t piece = 1ULL << (x + y * 8);
    return whitePieces & piece;
}

bool ChessBoard::removePieceAt(int x, int y) {
    if (isPieceAt(x, y)) {
        uint64_t piece = 1ULL << (x + y * 8);
        whitePieces &= ~piece;
        blackPieces &= ~piece;
        for (int i = 0; i < 6; ++i) {
            pieces[i] &= ~piece;
        }
        return true;
    }
    return false;
}

PieceType ChessBoard::getPieceTypeAt(int x, int y) const {
    if (isPieceAt(x, y, PAWN)) return PAWN;
    if (isPieceAt(x, y, ROOK)) return ROOK;
    if (isPieceAt(x, y, KNIGHT)) return KNIGHT;
    if (isPieceAt(x, y, BISHOP)) return BISHOP;
    if (isPieceAt(x, y, QUEEN)) return QUEEN;
    if (isPieceAt(x, y, KING)) return KING;
    return EMPTY;
}

bool ChessBoard::movePiece(int x, int y, int newX, int newY) {
    if (isPieceAt(x, y)) {
        if (isValidAttack(x, y, newX, newY) && isPieceAt(newX, newY, !getPieceColor(x, y))) {
            auto piece = getPieceTypeAt(x, y);
            removePieceAt(newX, newY);
            setPiece(newX, newY, piece, getPieceColor(x, y));
            removePieceAt(x, y);

            return true;
        } else if (isValidMove(x, y, newX, newY)) {
            auto piece = getPieceTypeAt(x, y);
            setPiece(newX, newY, piece, getPieceColor(x, y));
            removePieceAt(x, y);

            return true;
        } 
    }
    return false;
}

void ChessBoard::undoMove(int x, int y, int newX, int newY, PieceType capturedPiece) {
    bool color = getPieceColor(newX, newY);
    auto piece = getPieceTypeAt(newX, newY);
    setPiece(x, y, piece, color);
    removePieceAt(newX, newY);
    if (capturedPiece != EMPTY) {
        setPiece(newX, newY, capturedPiece, !color);
    }
}

uint64_t ChessBoard::getValidMoves(int x, int y) {
    uint64_t validMoves = 0;
    int64_t emptySquares = ~getBoard();

    // loop over all empty squares and check if the piece can move there
    while (emptySquares) {
        int index = __builtin_ctzll(emptySquares);
        int newX = index % 8;
        int newY = index / 8;

        if (isValidMove(x, y, newX, newY)) {
            validMoves |= 1ULL << index;
        }

        emptySquares &= emptySquares - 1;
    }

    return validMoves;
}

uint64_t ChessBoard::getValidAttacks(int x, int y) {
    uint64_t validAttacks = 0;
    bool color = getPieceColor(x, y);
    int64_t enemyPieces = getBoard(!color);

    // loop over all enemy pieces and check if the piece can attack them
    while (enemyPieces) {
        int index = __builtin_ctzll(enemyPieces);
        int enemyX = index % 8;
        int enemyY = index / 8;

        if (isValidAttack(x, y, enemyX, enemyY)) {
            validAttacks |= 1ULL << index;
        }

        enemyPieces &= enemyPieces - 1;
    }

    return validAttacks;
}


bool ChessBoard::isValidMove(int x, int y, int newX, int newY) const {
    if (isPieceAt(x, y)) {
        auto piece = getPieceTypeAt(x, y);
        auto pieceColor = getPieceColor(x, y);
        // Can't move to a square occupied by a piece of the same color
        if (isPieceAt(newX, newY, pieceColor)) {
            return false;
        }

        bool isWhite = isPieceAt(x, y, true);
        // Direction of movement for white and black pieces
        int direction = isWhite ? -1 : 1; 

        switch (piece) {
            case PAWN:
                if (newX == x && newY == y + direction) {
                    if (!isPieceAt(x, y + direction)) {
                        return true;
                    }
                } else if (newX == x && newY == y + 2 * direction && y == (isWhite ? 6 : 1)) {
                    if (!isPieceAt(x, y + direction) && !isPieceAt(x, y + 2 * direction)) {
                        return true;
                    }
                }
                break;
            case KNIGHT:
                // knight can move in an L shape
                return ((newX == x + 1 && newY == y + 2) || (newX == x + 2 && newY == y + 1) ||
                        (newX == x + 2 && newY == y - 1) || (newX == x + 1 && newY == y - 2) ||
                        (newX == x - 1 && newY == y - 2) || (newX == x - 2 && newY == y - 1) ||
                        (newX == x - 2 && newY == y + 1) || (newX == x - 1 && newY == y + 2));
                break;
            case QUEEN:
                if (newX == x || newY == y || newX - x == newY - y || newX - x == y - newY) {
                    // Check for blocking pieces in the path of the queen
                    if (!isPathClear(x, y, newX, newY)) {
                        return false;
                    }
                    return true;
                }
                break;
            case ROOK:
                // rook can move any number of spaces horizontally or vertically
                if (newX == x || newY == y) {
                    if (!isPathClear(x, y, newX, newY)) {
                        return false;
                    }
                    return true;
                }
                break;
            case BISHOP:
                // bishop can move any number of spaces diagonally
                if (newX - x == newY - y || newX - x == y - newY) {
                    if (!isPathClear(x, y, newX, newY)) {
                        return false;
                    }
                    return true;
                }
                break;
            case KING:
                // King can move 1 space in any direction
                if (std::abs(newX - x) <= 1 && std::abs(newY - y) <= 1) {
                    return true;
                }
                break;
        }
    }
    return false;
}

bool ChessBoard::isPathClear(int startX, int startY, int endX, int endY) const {
    // Check if the path from start to end is clear (no pieces in the way)
    int deltaX = (endX > startX) ? 1 : ((endX < startX) ? -1 : 0);
    int deltaY = (endY > startY) ? 1 : ((endY < startY) ? -1 : 0);

    int currentX = startX + deltaX;
    int currentY = startY + deltaY;

    while (currentX != endX || currentY != endY) {
        if (isPieceAt(currentX, currentY)) {
            return false;  // There is a piece in the path
        }
        currentX += deltaX;
        currentY += deltaY;
    }

    return true;  // The path is clear
}

bool ChessBoard::isValidAttack(int x, int y, int newX, int newY) const {
    if (isPieceAt(x, y)) {
        auto piece = getPieceTypeAt(x, y);
        // Can't attack pieces of the same color
        if (isPieceAt(x, y, true) == isPieceAt(newX, newY, true)) {
            return false;
        }

        bool isWhite = isPieceAt(x, y, true);
        // Direction of movement for white and black pieces
        int direction = isWhite ? -1 : 1;  

        switch (piece) {
            case PAWN:
                // Pawn attacks diagonally
                if ((newX == x + 1 || newX == x - 1) && newY == y + direction) {
                    return true;
                }
                break;
            default:
                // For other pieces, use the isValidMove function
                return isValidMove(x, y, newX, newY);
        }
    }
    return false;
}
