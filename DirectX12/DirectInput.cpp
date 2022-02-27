#include "DirectInput.h"

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

// ����}�N��
#define Release(X) { if((X) != nullptr) (X)->Release(); (X) = nullptr; }

// �R���X�g���N�^
Input::Input(HWND hwnd) 
    : 
    result(S_OK), 
    input(nullptr), 
    key(nullptr),
    m_hwnd(hwnd)
{
    memset(&keys, 0, sizeof(keys));
    memset(&olds, 0, sizeof(olds));

    CreateInput();
    CreateKey();
    SetKeyFormat();
    SetKeyCooperative();
}
// �f�X�g���N�^
Input::~Input()
{
    Release(key);
    Release(input);
}
// �C���v�b�g�̐���
HRESULT Input::CreateInput(void)
{
    result = DirectInput8Create(GetModuleHandle(0), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)(&input), NULL);

    return result;
}
// �L�[�f�o�C�X�̐���
HRESULT Input::CreateKey(void)
{
    result = input->CreateDevice(GUID_SysKeyboard, &key, NULL);

    return result;
}
// �L�[�t�H�[�}�b�g�̃Z�b�g
HRESULT Input::SetKeyFormat(void)
{
    result = key->SetDataFormat(&c_dfDIKeyboard);

    return result;
}
// �L�[�̋������x���̃Z�b�g
HRESULT Input::SetKeyCooperative(void)
{
    result = key->SetCooperativeLevel(m_hwnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);

    //���̓f�o�C�X�ւ̃A�N�Z�X�������擾
    key->Acquire();

    return result;
}
// �L�[����
bool Input::CheckKey(UINT index)
{
    //�`�F�b�N�t���O
    bool flag = false;

    //�L�[�����擾
    key->GetDeviceState(sizeof(keys), &keys);
    if (keys[index] & 0x80)
    {
        flag = true;
    }
    olds[index] = keys[index];

    return flag;
}
// �g���K�[�̓���
bool Input::TriggerKey(UINT index)
{
    //�`�F�b�N�t���O
    bool flag = false;

    //�L�[�����擾
    key->GetDeviceState(sizeof(keys), &keys);
    if ((keys[index] & 0x80) && !(olds[index] & 0x80))
    {
        flag = true;
    }
    olds[index] = keys[index];

    return flag;
}