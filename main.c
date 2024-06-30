#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ini.h"

#define Pi 3.141592
#define rpm_to_omega 0.104719
#define omega_to_rpm 9.549296

#define params_count 10
#define MAX_LEN 500

typedef struct
{
	double rpm;
	double v_inf;
	int nblades;
	double diameter;
	double radius_hub;
	const char* section;
	const char* radius_str;
	double radius;
	const char* chord_str;
	double chord;
	const char* pitch_str;
	double pitch;
	double rho;
	double mu;
} configuration;

static int handler(void* user, const char* section, const char* name,
	const char* value)
{
	configuration* pconfig = (configuration*)user;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
	if (MATCH("case", "rpm")) {
		pconfig->rpm = atof(value);
	}
	else if (MATCH("case", "v_inf")) {
		pconfig->v_inf = atof(value);
	}
	else if (MATCH("rotor", "nblades")) {
		pconfig->nblades = atoi(value);
	}
	else if (MATCH("rotor", "diameter")) {
		pconfig->diameter = atof(value);
	}
	else if (MATCH("rotor", "radius_hub")) {
		pconfig->radius_hub = atof(value);
	}
	else if (MATCH("rotor", "section")) {
		pconfig->section = _strdup(value);
	}
	else if (MATCH("rotor", "radius")) {
		pconfig->radius_str = _strdup(value);
	}
	else if (MATCH("rotor", "chord")) {
		pconfig->chord_str = _strdup(value);
	}
	else if (MATCH("rotor", "pitch")) {
		pconfig->pitch_str = _strdup(value);
	}
	else if (MATCH("fluid", "rho")) {
		pconfig->rho = atof(value);
	}
	else if (MATCH("fluid", "mu")) {
		pconfig->mu = atof(value);
	}
	else {
		return 0;  /* unknown section/name, error */
	}
	return 1;
}

double Get_Ct(double sigma, double Cl_alpha, double theta)
{
	double Ct1 = 0.1;
	double Ct2 = 0.0;

	while (1)
	{
		Ct2 = 0.5 * sigma * Cl_alpha * ((theta / 3) - (0.5 * sqrt(Ct1 / 2.0)));
		//printf("Ct1 = %f\n", Ct1);
		//printf("Ct2 = %f\n", Ct2);

		if (fabs(Ct1 - Ct2) < 10e-6 && Ct1 > 0)
			break;
		else
			Ct1 = Ct2;
	}

	return Ct1;
}

int main()
{
	configuration config;

	if (ini_parse("rotor.ini", handler, &config) < 0) {
		printf("로터 ini 파일 불러오기 오류.\n");
		return 0;
	}

	ini_parse("rotor.ini", handler, &config);

	printf("[case]\nrpm : %.2f\nv_inf = %.2f\n\n", config.rpm, config.v_inf);
	printf("[rotor]\nnblades = %d\ndiameter = %.2f\nradius_hub = %.2f\nsection = %s\nradius = %s\nchord = %s \npitch = %s\n\n",
		config.nblades, config.diameter, config.radius_hub, config.section, config.radius_str, config.chord_str, config.pitch_str);
	printf("[fulid]\nrho = %.2f\nmu = %.2f\n\n", config.rho, config.mu);



	////////////////////
	/// 익형 불러오기 ///
	////////////////////
	int section_count = 0;

	char* context = NULL;
	char* params = strtok_s(config.section, " ", &context);
	//printf("sections = %s\n", sections);

	char** section = (char**)malloc(sizeof(char*) * params_count);
	if (section == NULL)
	{
		printf("section 메모리 할당 오류.\n");
		return 0;
	}

	section[0] = params;
	section_count++;
	printf("section[0] = %s\n", section[0]);

	for (int i = 1; params != NULL; i++)
	{
		params = strtok_s(NULL, " ", &context);

		if (params != NULL)
		{
			section[i] = params;
			section_count++;

			//printf("sections = %s\n", sections);
			printf("section[%d] = %s\n", i, section[i]);
		}
	}
	printf("\n");

	if (params_count != section_count)
	{
		printf("에어포일 섹션 개수 오류.\n");
		return 0;
	}



	//////////////////////
	/// radius 불러오기 ///
	//////////////////////
	context = NULL;
	params = strtok_s(config.radius_str, " ", &context);

	int radius_count = 0;

	double* radius = (double*)malloc(sizeof(double) * params_count);
	if (radius == NULL)
	{
		printf("radius 메모리 할당 실패.\n");
		return 0;
	}
	radius[0] = atof(params);
	printf("radius[0] = %.2f m\n", radius[0]);
	radius_count++;

	for (int i = 1; params != NULL; i++)
	{
		params = strtok_s(NULL, " ", &context);

		if (params != NULL)
		{
			radius[i] = atof(params);
			radius_count++;

			printf("radius[%d] = %.2f m\n", i, radius[i]);
		}
	}
	printf("\n");

	if (params_count != radius_count)
	{
		printf("radius 섹션 개수 오류.\n");
		return 0;
	}



	/////////////////////
	/// chord 불러오기 ///
	/////////////////////
	context = NULL;
	params = strtok_s(config.chord_str, " ", &context);

	int chord_count = 0;

	double* chord = (double*)malloc(sizeof(double) * params_count);
	if (chord == NULL)
	{
		printf("chord 메모리 할당 실패.\n");
		return 0;
	}
	chord[0] = atof(params);
	printf("chord[0] = %.2f m\n", chord[0]);
	chord_count++;

	for (int i = 1; params != NULL; i++)
	{
		params = strtok_s(NULL, " ", &context);

		if (params != NULL)
		{
			chord[i] = atof(params);
			chord_count++;

			printf("chord[%d] = %.2f m\n", i, chord[i]);
		}
	}
	printf("\n");

	if (params_count != chord_count)
	{
		printf("chord 섹션 개수 오류.\n");
		return 0;
	}



	/////////////////////
	/// pitch 불러오기 ///
	/////////////////////
	context = NULL;
	params = strtok_s(config.pitch_str, " ", &context);

	int pitch_count = 0;

	double* pitch = (double*)malloc(sizeof(double) * params_count);
	if (pitch == NULL)
	{
		printf("pitch 메모리 할당 실패.\n");
		return 0;
	}
	pitch[0] = atof(params);
	printf("pitch[0] = %.2f ˚\n", pitch[0]);
	pitch_count++;

	for (int i = 1; params != NULL; i++)
	{
		params = strtok_s(NULL, " ", &context);

		if (params != NULL)
		{
			pitch[i] = atof(params);
			pitch_count++;

			printf("pitch[%d] = %.2f ˚\n", i, pitch[i]);
		}
	}
	printf("\n");

	if (params_count != pitch_count)
	{
		printf("pitch 섹션 개수 오류.\n");
		return 0;
	}



	/////////////////
	/// dy 구하기 ///
	/////////////////
	double dy_count = params_count - 1;
	double* dy = (double*)malloc(sizeof(double) * (dy_count));

	for (int i = 0; i < dy_count; i++)
	{
		dy[i] = chord[i + 1] - chord[i];
		printf("dy[%d] = %.2f m\n", i, dy[i]);
	}
	printf("\n");



	///////////////////////
	/// 로터 면적 구하기 ///
	///////////////////////
	double A_blade = 0.0;
	for (int i = 0; i < dy_count; i++)
		A_blade += chord[i] * dy[i];
	printf("A_blade = %f m²\n", A_blade);

	double A_disk = pow(config.diameter / 2.0, 2) * Pi;
	printf("A_disk = %f m²\n", A_disk);

	double sigma = A_blade / A_disk;
	printf("sigma = %f\n", sigma);
	printf("\n");



	///////////////////////
	/// Cl_alpha 구하기 ///
	///////////////////////
	double Cl_alpha = 2 * Pi;



	/////////////////
	/// Ct 구하기 ///
	/////////////////
	double Ct = Get_Ct(sigma, Cl_alpha, pitch[0]);
	printf("Ct = %f\n", Ct);
	printf("\n");


	////////////////////
	/// alpha 구하기 ///
	////////////////////
	double omega = config.rpm * rpm_to_omega;

	double* alpha = (double*)malloc(sizeof(double) * params_count);
	double* phi = (double*)malloc(sizeof(double) * params_count);
	double* UT = (double*)malloc(sizeof(double) * params_count);
	double* UP = (double*)malloc(sizeof(double) * params_count);
	double* U = (double*)malloc(sizeof(double) * params_count);

	for (int i = 0; i < params_count; i++)
	{

		UT[i] = omega * radius[i];
		UP[i] = UT[i] * 0.03;
		U[i] = sqrt(pow(UT[i], 2) + pow(UP[i], 2));

		phi[i] = atan(UP[i] / UT[i]);

		alpha[i] = pitch[i] - phi[i];
		printf("alpha[%d] = %.2f ˚\n", i, alpha[i]);
	}
	printf("\n");



	///////////////////////
	/// Reynolds 구하기 ///
	///////////////////////
	double* reynolds = (double*)malloc(sizeof(double) * params_count);

	for (int i = 0; i < params_count; i++)
	{
		reynolds[i] = (config.rho * omega * radius[i] * chord[i]) / config.mu;
		printf("reynolds[%d] = %.2f\n", i, reynolds[i]);
	}
	printf("\n");



	///////////////////////////
	///// LookUp Table 읽기 ///
	///////////////////////////
	//FILE* fp;
	//fp = fopen_s(&fp, "test.txt","r");
	//if (fp == NULL)
	//{
	//	printf("LookUp Table 불러오기 오류.\n");
	//	return 0;
	//}
	//else
	//{
	//	
	//}
	//fclose(fp);

	///////////////////////
	///// Cl, Cd 구하기 ///
	///////////////////////
	//double** reynolds_arr = (double**)malloc(sizeof(double*) * params_count);

	//for (int i = 0; i < params_count; i++)
	//{
	//	reynolds_arr[i] = 0;
	//}


	return 0;
}