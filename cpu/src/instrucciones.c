#include "instrucciones.h"

bool ejecutando_un_proceso;
t_registros_cpu registros_cpu;
int tid;

// Ciclo de instrucciones

char *fetch()
{
    log_info(logger_cpu, "TID: <%d> - FETCH - Program Counter: <%d>", tid, registros_cpu.PC);
    t_paquete *paquete = crear_paquete(FETCH_INSTRUCCION);
    // agregar_a_paquete_solo(paquete, &pid, sizeof(int));
    agregar_a_paquete_solo(paquete, &tid, sizeof(int));
    // agregar_a_paquete_solo(paquete, &program_counter, sizeof(int));
    agregar_a_paquete_solo(paquete, &registros_cpu.PC, sizeof(uint32_t));

    enviar_paquete(paquete, socket_memoria);

    char *instruccion_a_ejecutar = recibir_instruccion(socket_memoria);
    // program_counter++;
    registros_cpu.PC++;
    log_info(logger_cpu, "Program Counter actualizado: %d", registros_cpu.PC);
    eliminar_paquete(paquete);
    return instruccion_a_ejecutar;
}

instruccion *decode(char *inst)
{
    instruccion *instruccion_decodificada = malloc(sizeof(instruccion));

    char **vector_terminos = string_split(inst, " ");

    u_int8_t identificador = get_identificador(vector_terminos[0]);

    // printf("IDENTIFICADOR INSTRUCCION: %i\n", identificador);

    for (int i = 0; i < string_array_size(vector_terminos); i++)
    {
        free(vector_terminos[i]);
    }

    free(vector_terminos);

    instruccion_decodificada->identificador = identificador;
    instruccion_decodificada->cant_parametros = get_cant_parametros(identificador); // ver esta funcion
    instruccion_decodificada->parametros = get_parametros(inst, instruccion_decodificada->cant_parametros);
    free(inst);
    return instruccion_decodificada;
}

instruccion *execute(instruccion *inst)
{
    switch (inst->identificador)
    {
    case SET:
        mostrar_parametros(inst, inst->cant_parametros);
        set(inst);
        break;
    case SUM:
        mostrar_parametros(inst, inst->cant_parametros);
        sum(inst);
        break;
    case SUB:
        mostrar_parametros(inst, inst->cant_parametros);
        sub(inst);
        break;
    case JNZ:
        mostrar_parametros(inst, inst->cant_parametros);
        jnz(inst);
        break;
    case READ_MEM:
        mostrar_parametros(inst, inst->cant_parametros);
        read_mem(inst);
        break;
    case WRITE_MEM:
        mostrar_parametros(inst, inst->cant_parametros);
        write_mem(inst);
        break;
    case LOG:
        mostrar_parametros(inst, inst->cant_parametros);
        log_instruccion(inst);
        break;
    case DUMP_MEMORY:
        mostrar_parametros(inst, inst->cant_parametros);
        dump_memory(inst);
        break;
    case IO:
        mostrar_parametros(inst, inst->cant_parametros);
        io(inst);
        break;
    case PROCESS_CREATE:
        mostrar_parametros(inst, inst->cant_parametros);
        process_create(inst);
        break;
    case THREAD_CREATE:
        mostrar_parametros(inst, inst->cant_parametros);
        thread_create(inst);
        break;
    case THREAD_JOIN:
        mostrar_parametros(inst, inst->cant_parametros);
        thread_join(inst);
        break;
    case THREAD_CANCEL:
        mostrar_parametros(inst, inst->cant_parametros);
        thread_cancel(inst);
        break;
    case MUTEX_CREATE:
        mostrar_parametros(inst, inst->cant_parametros);
        mutex_create(inst);
        break;
    case MUTEX_LOCK:
        mostrar_parametros(inst, inst->cant_parametros);
        mutex_lock(inst);
        break;
    case MUTEX_UNLOCK:
        mostrar_parametros(inst, inst->cant_parametros);
        mutex_unlock(inst);
        break;
    case THREAD_EXIT:
        mostrar_parametros(inst, inst->cant_parametros);
        thread_exit(inst);
        break;
    case PROCESS_EXIT:
        mostrar_parametros(inst, inst->cant_parametros);
        process_exit(inst);
        break;
    case NO_RECONOCIDO:
        log_info(logger_cpu, "se leyo mal el identificador");
        ejecutando_un_proceso = false;
        break;
    default:
        break;
    }
    return inst;
}

// INSTRUCCIONES

// SET (Registro, Valor): Asigna al registro el valor pasado como parámetro
void set(instruccion *inst)
{
    uint32_t *dir_registro = get_direccion_registro(list_get(inst->parametros, 0));
    *dir_registro = atoi(list_get(inst->parametros, 1));
}

// SUM (Registro Destino, Registro Origen): Suma al Registro Destino el Registro Origen y deja el resultado en el Registro Destino.
void sum(instruccion *inst)
{
    char *registro_destino = list_get(inst->parametros, 0);
    char *registro_origen = list_get(inst->parametros, 1);

    uint32_t *registro_des = get_direccion_registro(registro_destino); // direccion del registro destino
    uint32_t *registro_ori = get_direccion_registro(registro_origen);  // dreccion del registro origen

    uint32_t aux = *registro_des + *registro_ori;

    *registro_des = aux;
}

// SUB (Registro Destino, Registro Origen): Resta al Registro Destino el Registro Origen y deja el resultado en el Registro Destino.
void sub(instruccion *inst)
{
    char *registro_destino = list_get(inst->parametros, 0);
    char *registro_origen = list_get(inst->parametros, 1);

    uint32_t *registro_des = get_direccion_registro(registro_destino); // direccion del registro destino
    uint32_t *registro_ori = get_direccion_registro(registro_origen);  // direccion del registro origen

    uint32_t aux = *registro_des - *registro_ori;

    *registro_des = aux;
}

// JNZ (Registro, Instrucción): Si el valor del registro es distinto de cero,
// actualiza el program counter al número de instrucción pasada por parámetro.
void jnz(instruccion *inst)
{
    uint32_t *dir_registro = get_direccion_registro(list_get(inst->parametros, 0));
    char *pcf = list_get(inst->parametros, 1);

    if (*dir_registro != 0)
    {
        int program_counter_futuro = atoi(pcf);
        registros_cpu.PC = program_counter_futuro;
    }
}

// READ_MEM (Registro Datos, Registro Dirección): Lee el valor de memoria correspondiente a la Dirección Lógica
// que se encuentra en el Registro Dirección y lo almacena en el Registro Datos.
void read_mem(instruccion *inst)
{
}

// WRITE_MEM (Registro Dirección, Registro Datos): Lee el valor del Registro Datos y lo escribe en la dirección
// física de memoria obtenida a partir de la Dirección Lógica almacenada en el Registro Dirección.
void write_mem(instruccion *inst)
{
}

// LOG (Registro): Escribe en el archivo de log el valor del registro.
void log_instruccion(instruccion *inst)
{
}

// SYSCALLS

void dump_memory(instruccion *inst)
{
}

void io(instruccion *inst)
{
}

void process_create(instruccion *inst)
{
}

void thread_create(instruccion *inst)
{
}

void thread_join(instruccion *inst)
{
}

void thread_cancel(instruccion *inst)
{
}

void mutex_create(instruccion *inst)
{
}

void mutex_lock(instruccion *inst)
{
}

void mutex_unlock(instruccion *inst)
{
}

void thread_exit(instruccion *inst)
{
}

void process_exit(instruccion *inst)
{
}

// PARAMETROS

/*void mostrar_parametros(instruccion *inst, int cant_parametros)
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
}*/

//////////////////////////////////////////////////////// ESTAS FUNCIONES PODRIAN IR A UN UTILS DE CPU