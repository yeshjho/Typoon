[![ko](https://img.shields.io/badge/lang-ko-blue)](https://github.com/yeshjho/Typoon/blob/main/README.md)
[![en](https://img.shields.io/badge/lang-en-red)](https://github.com/yeshjho/Typoon/blob/main/readme/README-en.md)

# <img src="https://raw.githubusercontent.com/yeshjho/Typoon/main/icon.ico" width="40"> Typoon

![GitHub release (latest by date)](https://img.shields.io/github/v/release/yeshjho/Typoon)
![GitHub release (latest by date including pre-releases)](https://img.shields.io/github/v/release/yeshjho/Typoon?include_prereleases)
![GitHub license](https://img.shields.io/github/license/yeshjho/Typoon)

![Cover Gif](/readme/cover.gif)

> 한글을 제대로 지원하는 텍스트 확장기

**Typoon**은 한글을 제대로 처리하도록 고안된 텍스트 확장기입니다.
시중에 나와 있는 대부분의 텍스트 확장기는 한국어 입력 시스템을 제대로 지원하지 않아 직접 만들게 되었습니다.
당연히 알파벳(영문) 또한 지원합니다.

이 프로그램은 아주 훌륭한 텍스트 확장기인 [espanso](https://espanso.org/)에게서 크게 영감을 받았습니다. 한글을 쓰지 않으신다면 Typoon 대신 espanso를 사용하시는 것을 추천드립니다.

## 설치 방법
[최신 배포](https://github.com/yeshjho/Typoon/releases/latest)로 들어가서 설치 파일을 다운받으세요.

## 사용 방법
[위키](https://github.com/yeshjho/Typoon/wiki/%EC%82%AC%EC%9A%A9-%EB%B0%A9%EB%B2%95)를 참고하세요.

## 기능
- 이미지 대치: 텍스트를 이미지로 대체할 수 있습니다.

- 명령어 실행: 텍스트를 명령어 실행 아웃풋으로 대체할 수 있습니다.

- 파일 변화 감지: 설정 파일이나 매치 파일의 변화를 자동으로 감지해 적용시킵니다.

- 커서 위치 지정: 대치 텍스트에서 커서의 위치를 맨 끝이 아닌 다른 곳으로 지정할 수 있습니다.

- JSON5: 설정 파일과 매치 파일은 [JSON5](https://json5.org/) 포맷을 사용합니다. 이 프로젝트는 JSON5의 [C++ 구현체](https://github.com/P-i-N/json5)를 사용합니다. JSON5를 모르시는 분들께 간략히 소개해드리자면, 사람이 읽고 쓰기가 좀 더 쉬운 JSON입니다.

### 옵션
#### 한글에만 적용되는 옵션
- 완전 조합: 글자의 조합이 끝나야지만 발동되도록 합니다.
- 조합 중 상태로 남기기: 대체된 텍스트의 마지막 글자를 조합 중인 상태가 되도록 합니다.

#### 알파벳에만 적용되는 옵션
- 대소문자 구별
- 대소문자 전파
    - 단어마다 대문자화

#### 둘 다 적용되는 옵션
- 단어: 단어 단위로 끊겨야지만 발동되도록 합니다.

이 옵션들의 구체적인 설명과 예시를 확인하려면 [위키](https://github.com/yeshjho/Typoon/wiki/%EC%82%AC%EC%9A%A9-%EB%B0%A9%EB%B2%95#%EC%98%B5%EC%85%98)를 참고하세요.

## 한계
### 키보드 레이아웃
현재 Typoon은 한글은 **두벌식**, 알파벳은 **QWERTY** 방식을 사용해야지만 올바르게 작동합니다.

### 플랫폼
현재 Typoon은 윈도우 전용입니다. 하지만 코드 설계 시 멀티 플랫폼 지원을 고려하긴 했으므로 다른 플랫폼에 대한 지원을 추가할 때 그렇게 힘들진 않을 것으로 예상됩니다. 그렇다 해도 제가 다른 플랫폼을 사용하고 있진 않아서 우선순위는 많이 낮습니다. 기여는 언제나 환영입니다!

## 향후 계획
[깃허브 프로젝트](https://github.com/users/yeshjho/projects/2)에서 계획된 기능들을 확인하실 수 있습니다. 하지만 저는 아직 학생이고 이 프로젝트는 취미이기 때문에, 마감일이 설정된 정확한 로드맵 같은 것은 드리기 힘듭니다. 기여는 언제나 환영입니다!

## 특별히 감사한 분들
- 황인철: 로고 제작, 프로그램 이름, 기타 몇몇 결정을 도와주었습니다.
