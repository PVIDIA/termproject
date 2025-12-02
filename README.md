Course: POSTECH Computer Graphics (2025 Autumn)
Project: Assignment 3-2: 3D Drawing
Author: 구현웅 (20210940, khw0410s)
Email: khw0410s@postech.ac.kr
Date: 2025-11-22

- Build & Run
build.bat으로 게임을 빌드 및 테스트 실행
run.bat으로 게임을 실행

- Controls
이동 : 방향키
발사 : Z
카메라 변환 : Q
렌더링 스타일 : W
충돌 범위 표시 : E
카메라 이동 : i(상),k(하),j(좌),l(우),u(앞),o(뒤)

- Project Structure
assets/                   # assets
build/                    # 빌드 폴더
src/
  main.cpp                # main
  model.h/ .cpp           # obj 로드 및 model draw
  node.h/ .cpp            # scene graph의 기본 노드
  shader.h                # shader 초기화 및 코드 정의
  game/
    MainScene.h/ .cpp     # Main Scene 및 game object 구현
win-x64-msvc/             # OpenGL 관련 폴더
build.bat                 # 빌드 배치 파일
build.ps1                 # 빌드 스크립트
run.bat                   # 실행 배치 파일
