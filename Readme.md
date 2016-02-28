# KanColleSimulator

## �T�v
�͂���̐퓬���V�~�����[�g���܂��B

## �g����
`usage: KCS_CUI -i input1.json input2.json [-f formation1 formation2] [-n times] [-t threads] [-o output.csv]`

- input1.json�͎��͑��Ainput2.json�͓G�͑����}�b�v�̃f�[�^
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
- output.csv���w�肷��ƁA���ʂ�W���o�͂ł͂Ȃ�CSV�ɏ����o���悤�ɂȂ�B  
�܂��A�o�͌`�����W���o�͂̂悤�Ȋ���(�܂Ƃߕ\��)�ł͂Ȃ��e��ɂ��Ă̌��ʂ������o�����

## �g�p��
`KCS_CUI -i "sample\fleet sample1.json" "sample\fleet sample2.json" -f 0 1 -n 1`

`KCS_CUI -i "sample\fleet 1-5.json" "sample\map 1-5 high.json" -f 0 0 -n 10000 -t 2 -o output.csv`

`KCS_CUI -i "sample\fleet 2016�~E3�b�A��.json" "sample\map 2016�~E3�b�A��.json" -f 3 4 -n 100000 -t 4 -o output.csv`

## JSON�f�[�^�̏���(�͑���)
- �f�b�L�r���_�[�`�����Q�l�ɂ��A�@�\���g������
- �ʏ�͑����Ƒ�1�͑��A�A���͑����Ƒ�1�E��2�͑������Q�Ƃ��Ȃ�
- �uversion�v�Ɠ����K�w�Łulv�v(�i�ߕ����x��)��ݒu����  
(���ꂪ���݂��Ȃ��ꍇ�͎i�ߕ����x��120�Ƃ���)
- �uversion�v�Ɠ����K�w�Łutype�v(�͑�)��ݒu�����B1�Ȃ�ʏ�͑��A2�Ȃ�A���͑�  
(���ꂪ���݂��Ȃ��ꍇ�͒ʏ�͑��Ƃ���)
- �uluck�v�Ɠ����K�w�Łucond�v(cond�l)��ݒu����  
(���ꂪ���݂��Ȃ��ꍇ��cond�l49�Ƃ���)
- �urf�v�Ɠ����K�w�Łurf_detail�v(�����n���x)��ݒu����  
(���ꂪ���݂���ꍇ�́Arf�̐ݒ���㏑������)

## JSON�f�[�^�̏���(�}�b�v��)

## �\�[�X�R�[�h�̏���(C++��)
- �ϐ��̌^�́A��{�I��cstdint�̌^���g���悤�ɂ���B�܂��Aunsigned�𑽗p���Ȃ��悤�ɂ���
- �ϐ��̓X�l�[�N�P�[�X�����A�����o�ϐ��͖�����`_`��t����
- �萔���̓A�b�p�[�L�������P�[�X�̐擪��`k`��t����BCONST_NAME�Ƃ͏����Ȃ�
- �N���X������ъ֐����̓A�b�p�[�L�������P�[�X�Ƃ���
- getter�Esetter�̐ړ���ɂ͂��ꂼ��Get�ESet��t����悤�ɂ���
- �}�N���͔񐄏������A�p����ꍇ��MACRO_CODE�̂悤�ɂ���

## �I�v�V�����ŕύX�\�ȉӏ�(�\��)
- �͖��̃��x���̍ő�l
- ���G�v�Z�̃��\�b�h
- ���G�t�F�C�Y�̐�������

## ���ӓ_
- �⋭���݂ɂ͑Ή����Ă��܂���B
- �q���}�X�A����ы�P��}�X�ɂ͑Ή����Ă��܂���B
- �x���͑��ɂ͑Ή����Ă��܂���B

## ���C�Z���X
MIT���C�Z���X�Ƃ��܂��B

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
- ���ԑ���ɂ�h2suzuki�̃R�[�h�������������Ďg�p����  
http://qiita.com/h2suzuki/items/be367edcc8834b508a78
- JSON�ǂݍ��݂ɂ�picojson���g�p����
https://github.com/kazuho/picojson
- �͂���̎d�l�͒ʏ�wiki�E����wiki�Ȃǂ��Q�l�ɂ���
http://wikiwiki.jp/kancolle/
http://ja.kancolle.wikia.com/wiki/%E8%89%A6%E3%81%93%E3%82%8C_%E6%A4%9C%E8%A8%BCWiki
https://github.com/andanteyk/ElectronicObserver/blob/master/ElectronicObserver/Other/Information/kcmemo.md#%E8%89%A6%E3%81%93%E3%82%8C%E3%81%AE%E4%BB%95%E6%A7%98%E3%81%AB%E9%96%A2%E3%81%99%E3%82%8B%E9%9B%91%E5%A4%9A%E3%81%AA%E3%83%A1%E3%83%A2
