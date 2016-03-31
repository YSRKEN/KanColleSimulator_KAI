# KanColleSimulator

## �T�v
�͂���̐퓬���V�~�����[�g���܂��B

## �g����(GUI��)
- �͑��G�f�B�^�E�}�b�v�G�f�B�^�E�V�~�����[�g��3�̃^�u�ɕ�����Ă��܂�
- �͑��G�f�B�^���J���Ă���ۂ́A�͑��f�[�^(*.json)�����j���[����ǂݍ���or�h���b�O���h���b�v�ł��܂�
- �}�b�v�G�f�B�^���J���Ă���ۂ́A�}�b�v�f�[�^(*.map)�����j���[����ǂݍ���or�h���b�O���h���b�v�ł��܂�
- �^�̏����l��-1(�܂�f�t�H���g�l)�ɐݒ肳��Ă��܂�
- �O���n���x�Ɠ����n���x�͘A�����Ă��āA�Е���ύX����Ƃ����Е��������ŕω����܂�
- ��q����悤�ɁA�͑�vs�}�b�v�̏ꍇ�A���w�`�w���(�ΐ��}�X������)�J�����}�X�p�ŁA�G�w�`�w��͖�������܂�

## �g����(CUI��)
`usage: KCS_CUI -i input1.json input2.json|input2.map [-f formation1 formation2] [-n times] [-t threads] [-o output.json] [--result-json-prettify | --no-result-json-prettify]`

- input1.json�͎��͑��Ainput2.json|input2.map�͓G�͑����}�b�v�̃f�[�^�B  
��҂��}�b�v�̃f�[�^�Ȃ�}�b�v���[�h�A�����łȂ��Ȃ�ʏ탂�[�h�ƌĂ�
- ���͑��ɂ͘A���͑������e����邪�A�G�͑���}�b�v���̊͑��͒ʏ�͑��̂�
- formation1�͎��͑��Aformation2�͓G�͑��̐w�`�B
�@�P�c�w�E���c�w�E�֌`�w�E��`�w�E�P���w��0�`4�ɑΉ�����B  
�@�Ȃ��A�ȗ�����Ƃǂ�����P�c�w�ɂȂ�B
�@(�A���͑����ƁA���`��l�x���q�s����4�E1�E2�E0�ɑΉ�����)
- input2.json���}�b�v�̃f�[�^���ƁAformation2�ɏ����ꂽ�w�`�͖��������B  
�@�܂��A�}�b�v���[�h�ɂ����鎩�w�`�́A����or3�ǈȉ����ƒP�c�w�E  
�@�G���͂������͂���(�J�����}�X�܂߂�)�P���w�E  
�@�J�����}�X��(�ΐ��}�X��������)formation1�Ŏw�肵���w�`�ɂȂ�
- times�͎��s�񐔁B�l��1���Ɗ͑��̏ڍׂ��\�L�����B������1�ɂȂ�
- threads�͎��s�X���b�h���B�����ƃV���O���X���b�h�Ŏ��s�����
- ���ʂ͕W���o�͂ɏ����o�����B�}�b�v���[�h�Ȃ�A�����̓��B���E��j�P�ޗ����o��
- output.json���w�肷��ƁA���ʂ�W���o�͂����ł͂Ȃ�JSON�ɏ����o���悤�ɂȂ�B  
�Ȃ��A�}�b�v���[�h�ł́A�{�X�}�X�ł̌��ʂ̂�JSON�ɏ����o�����
- --no-result-json-prettify���w�肷��ƁA�o�͂���JSON�����`����Ȃ��悤�ɂȂ�

## �g�p��(CUI��)
`KCS_CUI -i "sample\sample1.json" "sample\sample2.json" -f 0 0 -n 1`

`KCS_CUI -i "sample\3-5.json" "sample\3-5 high.map" -f 0 0 -n 100000 -t 4 -o output.json`

## JSON�f�[�^�̏���(�͑���)
- UTF-8�ŕۑ����Ă�������(�ǂ���ASCII�������������Ǝv���܂���)
- �f�b�L�r���_�[�`�����Q�l�ɂ��A�@�\���g������
- �ʏ�͑����Ƒ�1�͑��A�A���͑����Ƒ�1�E��2�͑������Q�Ƃ��Ȃ�
- �uversion�v�Ɠ����K�w�Łulv�v(�i�ߕ����x��)��ݒu����  
(���ꂪ���݂��Ȃ��ꍇ�͎i�ߕ����x��120�Ƃ���)
- �uversion�v�Ɠ����K�w�Łutype�v(�͑�)��ݒu�����B1�Ȃ�ʏ�͑��A2�`4�Ȃ�A���͑�  
(���ꂪ���݂��Ȃ��ꍇ�͒ʏ�͑��Ƃ���)  
(�A���͑��́A���@�������E����Ō������E�A����q���������ꂼ��2�`4�ɑΉ�)
- �uluck�v�Ɠ����K�w�Łucond�v(cond�l)��ݒu����  
(���ꂪ���݂��Ȃ��ꍇ��cond�l49�Ƃ���)
- �urf�v�Ɠ����K�w�Łurf_detail�v(�����n���x)��ݒu����  
(���ꂪ���݂���ꍇ�́Arf�̐ݒ���㏑������)

## JSON�f�[�^�̏���(�}�b�v��)
- UTF-8�ŕۑ����Ă�������(�ǂ���ASCII�������������Ǝv���܂���)
- �u"version": "map"�v�͌��܂蕶��ł�
- �u"position": [�e�}�X�ɂ��Ă̏��̔z��]�v�ƂȂ��Ă��܂�
- �e�}�X�ɂ��Ă̏��́u"name":�}�X���v�u"pattern"�F�͑��̔z��v�u�퓬���[�h�v����Ȃ�܂�
- �͑��̔z��́A�u"fleets":�͑DID�̔z��v�u"form"�F�w�`�v����Ȃ�܂�
- �}�b�v�ɋ���͑D�̑����͏��������ł�(ships.csv���Q��)
- form�͐w�`�ŁA��L�ɂ�����formation�ƈӖ��͓����ł�
- mode�͐퓬���[�h�ŁA0������퓬(�ʏ�{�X�}�X�̂�)�E1������̂݁E2���J�����}�X�ł�
- �͑��̔z��́A�}�X�ɓ��ݍ��ނ��тɃ����_���őI�����邽�߂̂��̂ł�

## �\�[�X�R�[�h�̏���(C++��)
- �ϐ��̓X�l�[�N�P�[�X�����A�����o�ϐ��͖�����`_`��t����
- �萔���̓A�b�p�[�L�������P�[�X�̐擪��`k`��t����BCONST_NAME�Ƃ͏����Ȃ�
- �N���X������ъ֐����̓A�b�p�[�L�������P�[�X�Ƃ���
- getter�Esetter�̐ړ���ɂ͂��ꂼ��Get�ESet��t����悤�ɂ���
- �}�N���͔񐄏������A�p����ꍇ��MACRO_CODE�̂悤�ɂ���

## ���ӓ_
- �_���R������ѐ퓬���H�E�����͂ɂ��⋋�ɂ͑Ή����Ă��܂���B
- ���������āA�}�b�v���[�h�ł͑�j�����ەK���P�ނ��܂�
- ��q�ޔ��ɂ͑Ή����Ă��܂���
- �⋭���݂ɂ͑Ή����Ă��܂���
- �q���}�X�A����ы�P��}�X�ɂ͑Ή����Ă��܂���
- �x���͑��ɂ͑Ή����Ă��܂���

## �I�v�V�����ŕύX�\�ȉӏ�(�\��)
- �͖��̃��x���̍ő�l
- ���G�v�Z�̃��\�b�h
- ���G�t�F�C�Y�̐�������
- �G�ڊJ�n���̌v�Z���@(�J�n������ёI��)
- �󒆐�ɂ�����G�@�̌��ė�
- �΋�J�b�g�C���ɂ����锭���m��
- ���΂�����������m���A����т��΂��͂̑I����@
- �������v�Z��
- �t�B�b�g�C�␳�̌v�Z��
- �N���e�B�J��������
- �e��ʕ␳
- �e���ϑ��ˌ�(�J�b�g�C��)�������ɂ����閽�����␳
- �e���ϑ��ˌ��̔����m��
- �A���͑��ɂ����閽�����␳
- �T�Ɠ��ɂ��U����
- �d���ɂ����C���������␳

## ���C�Z���X
MIT���C�Z���X�Ƃ��܂��B

## �J����
|���O       |GitHub                        |Twitter                      |
|:----------|:-----------------------------|:----------------------------|
|YSR        |https://github.com/YSRKEN     |https://twitter.com/YSRKEN   |
|yumetodo   |https://github.com/yumetodo   |https://twitter.com/yumetodo |
|Cranberries|https://github.com/LoliGothick|https://twitter.com/_EnumHack|
|sayurin    |https://github.com/sayurin    |https://twitter.com/haxe     |
|taba       |https://github.com/taba256    |https://twitter.com/__taba__ |

## �o�[�W��������
### Ver.2.0.0
GUI��Visual C#�Ŏ��������B  
�R���p�C������csv�̃f�[�^����荞�ނ̂ŁACUI�ł������s����ۂ�csv���s�v�ɂȂ����B  
���G�v�Z����2-5��(�H)�����łȂ�33�������������B
### Ver.1.4.0
�΋�J�b�g�C������E�͑��h��{�[�i�X���L���b�V�����邱�Ƃŏ������x�����コ�����B
�܂��Aconstexpr�ɂ��R���p�C������CSV�f�[�^��ǂݍ��ނ��ƂŁA�L�q�̖��ĉ�����э��������ʂ������B
### Ver.1.3.0
��̃{�g���l�b�N���������āA�}�b�v���[�h��������ƕ��񓮍삷��悤�ɂ����B  
�܂��A�R��/�e�����ɔ����␳����萳�m�ɂ��A�G�ڂ��C�����A��ԐG�ڂ���ѐ���퓬�@�����������B  
�Ȃ��A�͑D�E�����f�[�^���ꕔ�X�V����Ă���B
### Ver.1.2.0
�}�b�v���[�h�����������B�܂��A��������ɂ��Ẵo�O���C�������B
### Ver.1.1.1
�������肪�F�X�Ƃ������������̂ŏC���B�܂��A�͑D�f�[�^����уT���v���f�[�^���X�V�����B
### Ver.1.1.0
�e��֐������������邱�Ƃ�Ver.1.0.1���{�͑����Ȃ����͂��B�܂��A�͑D�E�����f�[�^���X�V�����B
### Ver.1.0.1
���ɒ��񂾑���ɑ΂��Ēǂ��ł��������Ă��܂����Ƃ��������̂ŏC���B
### Ver.1.0.0
���܂Ŋ������܂����̂ŁA�Ƃ肠���������[�X���܂����B

## �Q�l����
- �ق��ۃA���t�@�̃f�[�^���C�u��������ships0209.csv��slotitems0209.csv�𒸂���  
(�\�t�g�E�F�A�p�ɓK�X�������Ă���)  
https://github.com/koshian2/HoppoAlpha.DataLibrary/tree/master/HoppoAlpha.DataLibrary
- �͎�L���͂��̃y�[�W���Q�l�ɂ���  
http://uswarships.jounin.jp/sub4.htm
- usage�̏����͂��̃y�[�W���Q�l�ɂ���  
http://www.fujlog.net/2014/04/command-line-getopt-usage-format.html
- Markdown�L�@�͂��̃T�C�g�ɏ����Ă���  
http://tatesuke.github.io/KanTanMarkdown/ktm-full.html
- C++�R�[�h�̏����́uGoogle C++ �X�^�C���K�C�h�v�ɏ����Ă���  
http://www.textdrop.net/google-styleguide-ja/cppguide.xml
- JSON�ǂݍ��݂ɂ�picojson���g�p����  
https://github.com/kazuho/picojson
- �͂���̎d�l�͒ʏ�wiki�E����wiki�Ȃǂ��Q�l�ɂ���  
http://wikiwiki.jp/kancolle/  
http://ja.kancolle.wikia.com/wiki/%E8%89%A6%E3%81%93%E3%82%8C_%E6%A4%9C%E8%A8%BCWiki  
https://github.com/andanteyk/ElectronicObserver/blob/master/ElectronicObserver/Other/Information/kcmemo.md#%E8%89%A6%E3%81%93%E3%82%8C%E3%81%AE%E4%BB%95%E6%A7%98%E3%81%AB%E9%96%A2%E3%81%99%E3%82%8B%E9%9B%91%E5%A4%9A%E3%81%AA%E3%83%A1%E3%83%A2  
http://kancollecalc.web.fc2.com/damage_formula.html  
http://bs-arekore.at.webry.info/201502/article_4.html  
https://twitter.com/Xe_UCH/status/705281106011029505  
http://ch.nicovideo.jp/biikame/blomaga/ar850895  
https://docs.google.com/spreadsheets/d/1O91DpCaHbjCZV2jy1GlMyqjwyWULdjK5kwhYALPFJGE/edit  
http://ch.nicovideo.jp/umya/blomaga/ar899278
https://twitter.com/noratako5/status/707526758434607104
- ���̂�SourceTree�c�c����ɓ���g���q�������ŏ��O����Ƃ́c�c  
http://tech.nitoyon.com/ja/blog/2013/04/05/sourcetree/
