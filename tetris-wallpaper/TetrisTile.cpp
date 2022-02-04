#include "TetrisTile.h"

TetrisTile::TetrisTile(SDL_Renderer* renderer, IconGrid* iconGrid, POINT pos, std::vector<POINT> blocks)
  : renderer(renderer), iconGrid(iconGrid), pos(pos), blocks(blocks) {
  // Random color
  color = HsvToRgb(((GetTimeInMS() / 100) % 0xff), 0xff, 0xff);
}

void TetrisTile::Render(bool iconVisible) {
  SDL_SetRenderDrawColor(renderer, iconVisible ? color.r : 0x0, iconVisible ? color.g : 0x0, iconVisible ? color.b : 0x0, iconVisible ? color.a : 0x0);

  for (auto& block : blocks) {
    POINT blockPos = { pos.x + block.x, pos.y + block.y };

    RECT tileRect;
    iconGrid->GetRect(blockPos, tileRect);
    SDL_Rect sdlRect;
    sdlRect.x = (int)tileRect.left;
    sdlRect.y = (int)tileRect.top;
    sdlRect.w = (int)tileRect.right - (int)tileRect.left;
    sdlRect.h = (int)tileRect.bottom - (int)tileRect.top;
    SDL_RenderFillRect(renderer, &sdlRect);

    iconGrid->SetVisible(blockPos, iconVisible);
  }
}

bool TetrisTile::Collides(std::shared_ptr<TetrisTile> other) {
  for (auto& block : blocks) {
    if (other->Collides({ pos.x + block.x, pos.y + block.y }))
      return true;
  }
  return false;
}
bool TetrisTile::Collides(POINT otherPos) {
  for (auto& thisBlock : blocks) {
    if (pos.x + thisBlock.x == otherPos.x && pos.y + thisBlock.y == otherPos.y)
      return true;
  }
  return false;
}

void TetrisTile::Rotate(bool left) {
  for (auto& block : blocks) {
    if (left) {
      long tmp = block.y;
      block.y = -block.x;
      block.x = tmp;
    } else {
      long tmp = block.y;
      block.y = block.x;
      block.x = -tmp;
    }
  }
}

void TetrisTile::Translate(POINT tr) {
  pos.x += tr.x;
  pos.y += tr.y;
}

std::shared_ptr<TetrisTile> TetrisTile::MakeRandomTile(SDL_Renderer* renderer, IconGrid* iconGrid, POINT pos) {
  std::vector<POINT> blocks;

  int randTile = rand() % 7;
  if (randTile == 0) {
    // ##
    //  O#
    blocks.push_back({ 0l, 0l });
    blocks.push_back({ 0l, -1l });
    blocks.push_back({ -1l, -1l });
    blocks.push_back({ 1l, 0l });
  } else if (randTile == 1) {
    // ##O#
    blocks.push_back({ 0l, 0l });
    blocks.push_back({ -1l, 0l });
    blocks.push_back({ -2l, 0l });
    blocks.push_back({ 1l, 0l });
  } else if (randTile == 2) {
    //  ##
    // #O
    blocks.push_back({ 0l, 0l });
    blocks.push_back({ 0l, -1l });
    blocks.push_back({ 1l, -1l });
    blocks.push_back({ -1l, 0l });
  } else if (randTile == 3) {
    //  #
    // #O#
    blocks.push_back({ 0l, 0l });
    blocks.push_back({ 0l, -1l });
    blocks.push_back({ -1l, 0l });
    blocks.push_back({ 1l, 0l });
  } else if (randTile == 4) {
    //   #
    // ##O
    blocks.push_back({ 0l, 0l });
    blocks.push_back({ -1l, 0l });
    blocks.push_back({ -2l, 0l });
    blocks.push_back({ 0l, -1l });
  } else if (randTile == 5) {
    // #
    // O##
    blocks.push_back({ 0l, 0l });
    blocks.push_back({ 1l, 0l });
    blocks.push_back({ 2l, 0l });
    blocks.push_back({ 0l, -1l });
  } else {
    // ##
    // O#
    blocks.push_back({ 0l, 0l });
    blocks.push_back({ 0l, -1l });
    blocks.push_back({ 1l, 0l });
    blocks.push_back({ 1l, -1l });
  }

  return std::make_shared<TetrisTile>(renderer, iconGrid, pos, blocks);
}
