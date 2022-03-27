// ���_�V�F�[�_�[����s�N�Z���V�F�[�_�[�ւ̂����Ɏg�p����\����

struct Output {
	float4 svpos : SV_POSITION; // system�p���_���W
	float4 normal : NORMAL;		// system�p�@�����W
	float2 uv : TEXCOORD;		// uv�l
};

Texture2D<float4> tex : register(t0); // 0�ԃX���b�g�ɐݒ肳�ꂽ�e�N�X�`��
SamplerState smp : register(s0);		  // 0�ԃX���b�g�ɐݒ肳�ꂽ�T���v���[

cbuffer cbuff0 : register(b0) // 0�ԃX���b�g�ɐݒ肳�ꂽ�萔
{
	matrix world; // ���[���h�ϊ��s��
	matrix viewproj; // ���[���h�ϊ��s��
}