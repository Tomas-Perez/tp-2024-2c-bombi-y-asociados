#include "utilsCpu.h"


void mostrar_parametros(instruccion *inst, int cant_parametros)
{
    if (cant_parametros == 0)
    {
        log_info(logger_cpu, "TID: <%u> - Ejecutando: <%s> - <>", tid, get_motivo(inst->identificador));
    }
    if (cant_parametros == 1)
    {
        char *param1 = list_get(inst->parametros, 0);
        log_info(logger_cpu, "TID: <%u> - Ejecutando: <%s> - <%s>", tid, get_motivo(inst->identificador), param1);
    }
    if (cant_parametros == 2)
    {
        char *param1 = list_get(inst->parametros, 0);
        char *param2 = list_get(inst->parametros, 1);
        log_info(logger_cpu, "TID: <%u> - Ejecutando: <%s> - <%s,%s>\n", tid, get_motivo(inst->identificador), param1, param2);
    }
    if (cant_parametros == 3)
    {
        char *param1 = list_get(inst->parametros, 0);
        char *param2 = list_get(inst->parametros, 1);
        char *param3 = list_get(inst->parametros, 2);
        log_info(logger_cpu, "TID: <%u> - Ejecutando: <%s> - <%s,%s,%s>\n", tid, get_motivo(inst->identificador), param1, param2, param3);
    }
}

char *get_termino(char *instruccion, int index)
{
    char **vector_terminos = string_split(instruccion, " ");
    return vector_terminos[index];
}

t_list *get_parametros(char *inst, u_int8_t cant_parametros)
{
    char **vector_terminos = string_split(inst, " ");
    t_list *parametros = list_create();
    for (int i = 1; i <= cant_parametros; i++)
    {
        char *parametro = malloc(strlen(vector_terminos[i]) + 1); //+1
        strcpy(parametro, vector_terminos[i]);
        list_add(parametros, parametro);
    }
    for (int i = 0; i < string_array_size(vector_terminos); i++)
    {
        free(vector_terminos[i]);
    }

    free(vector_terminos);
    return parametros;
}

u_int8_t get_identificador(char *identificador_leido)
{
    u_int8_t identificador = NO_RECONOCIDO;
    if (!strcmp("SET", identificador_leido))
        identificador = SET;
    if (!strcmp("SUM", identificador_leido))
        identificador = SUM;
    if (!strcmp("SUB", identificador_leido))
        identificador = SUB;
    if (!strcmp("JNZ", identificador_leido))
        identificador = JNZ;
    if (!strcmp("READ_MEM", identificador_leido))
        identificador = READ_MEM;
    if (!strcmp("WRITE_MEM", identificador_leido))
        identificador = WRITE_MEM;
    if (!strcmp("LOG", identificador_leido))
        identificador = LOG;
    if (!strcmp("DUMP_MEMORY", identificador_leido))
        identificador = DUMP_MEMORY;
    if (!strcmp("IO", identificador_leido))
        identificador = IO;
    if (!strcmp("PROCESS_CREATE", identificador_leido))
        identificador = PROCESS_CREATE;
    if (!strcmp("THREAD_CREATE", identificador_leido))
        identificador = THREAD_CREATE;
    if (!strcmp("THREAD_JOIN", identificador_leido))
        identificador = THREAD_JOIN;
    if (!strcmp("THREAD_CANCEL", identificador_leido))
        identificador = THREAD_CANCEL;
    if (!strcmp("MUTEX_CREATE", identificador_leido))
        identificador = MUTEX_CREATE;
    if (!strcmp("MUTEX_LOCK", identificador_leido))
        identificador = MUTEX_LOCK;
    if (!strcmp("MUTEX_UNLOCK", identificador_leido))
        identificador = MUTEX_UNLOCK;
    if (!strcmp("THREAD_EXIT", identificador_leido))
        identificador = THREAD_EXIT;
    if (!strcmp("PROCESS_EXIT", identificador_leido))
        identificador = PROCESS_EXIT;
    return identificador;
}

char *recibir_instruccion(int socket_cliente)
{
    int size = 0;
    recibir_operacion(socket_cliente);
    char *buffer = recibir_buffer(&size, socket_cliente);

    return buffer;
}

uint32_t *get_direccion_registro(char *string_registro)
{
    uint32_t *registro;

    if (!strcmp(string_registro, "AX"))
    {
        registro = &registros_cpu.AX;
    }
    if (!strcmp(string_registro, "BX"))
    {
        registro = &registros_cpu.BX;
    }
    if (!strcmp(string_registro, "CX"))
    {
        registro = &registros_cpu.CX;
    }
    if (!strcmp(string_registro, "DX"))
    {
        registro = &registros_cpu.DX;
    }
    if (!strcmp(string_registro, "EX"))
    {
        registro = &registros_cpu.EX;
    }
    if (!strcmp(string_registro, "FX"))
    {
        registro = &registros_cpu.FX;
    }
    if (!strcmp(string_registro, "GX"))
    {
        registro = &registros_cpu.GX;
    }
    if (!strcmp(string_registro, "HX"))
    {
        registro = &registros_cpu.DX;
    }
    if (!strcmp(string_registro, "PC"))
    {
        registro = &registros_cpu.PC;
    }

    return registro;
}

char *get_motivo(int motivo)
{
    char *identificadores[] = {
        "MENSAJE",
        "PAQUETE",
        "SET",
        "READ_MEM",
        "WRITE_MEM",
        "SUM",
        "SUB",
        "JNZ",
        "LOG",
        "DUMP_MEMORY",
        "IO",
        "PROCESS_CREATE",
        "THREAD_CREATE",
        "THREAD_JOIN",
        "THREAD_CANCEL",
        "MUTEX_CREATE",
        "MUTEX_LOCK",
        "MUTEX_UNLOCK",
        "THREAD_EXIT",
        "PROCESS_EXIT",
    };
    return identificadores[motivo];
}

u_int8_t get_cant_parametros(u_int8_t identificador)
{
    u_int8_t cant_parametros = 0;

    switch (identificador)
    {
    case DUMP_MEMORY:
    case THREAD_EXIT:
    case PROCESS_EXIT:
        break;
    case LOG:
    case IO:
    case THREAD_JOIN:
    case THREAD_CANCEL:
    case MUTEX_CREATE:
    case MUTEX_LOCK:
    case MUTEX_UNLOCK:
        cant_parametros = 1;
        break;
    case SET:
    case SUM:
    case SUB:
    case JNZ:
    case READ_MEM:
    case WRITE_MEM:
    case THREAD_CREATE:
        cant_parametros = 2;
        break;
    case PROCESS_CREATE:
        cant_parametros = 3;
        break;
    }
    return cant_parametros;
}