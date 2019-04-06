#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "FS.h"
     
const char* ssid = "LED";
const char* password = "12345678";
struct Config{
    String idBoton;
    String Descripcion;
    String Cantidad;
    String Precio;
    String tiempo;
};
String strPulsador;
String strPulsadorUltimo;

const char *filename = "/datos.json";
Config config;

static const char PROGMEM INDEX_HTML[] = R"( 
<!DOCTYPE html>
<html lang='es'>
    <head>
        <meta charset='UTF-8'>
        <meta name='viewport' content='width=device-width, initial-scale=1, shrink-to-fit=no'>
        <title>Proyecto</title>
    </head>
    <style>
    *{
        margin: 0;
        padding: 0;
        box-sizing: border-box;
        font-family:"Trebuchet MS", Helvetica, sans-serif;
    }
    body{
        color: #393939;
    }
    header{
        width: 100%;
        background-color: #1C1C1C;
        color: #F1F0F0;
        padding: 10px 0px;
        z-index: 1;
    }
    .container{
        width: 95%;
        margin: 0 auto;
    }
    .tabla{
        margin-top: 30px;
        width: 100%;
        text-align: center;
        font-size: 12px;
    }
    .tabla th{
        padding: 5px;
        background-color: firebrick;
        color: white;
        font-weight: 400;
    }
    .tabla td{
        padding: 10px 0px;
        border-bottom: 1px solid #D0D0D0;
    }
    .tabla tbody tr:nth-child(even){
        background-color: #f2f2f2;
    }
    .tabla tbody tr:hover{
        cursor: pointer;
        background-color: #D8D8D8;
    }
    button{
        margin-top: 25px;
        width: 100%;
        height: 40px;
        font-size: 15px;
        cursor: pointer;
        border: 1px solid #D0D0D0;
        transition: 0.3s;
    }
    button:hover{
        background-color: #AFAEAE;
    }
    .card{
        margin-top: 30px;
        width: 100%;
        border: 1px solid #D0D0D0;
        border-radius: 3px;
    }
    .card-title{
        padding: 10px;
        background-color: #EFEFEF;
    }
    .card-body{
        padding: 10px;
        background-color: white;
    }
    .card-body input{
        width: 100%;
        height: 30px;
        border-radius: 3px;
        border: 1px solid #D0D0D0;
        padding: 5px;
        margin-top: 15px;
    }
    .card-space{
        margin-bottom: 30px;
    }
    .modal{
        position: fixed;
        top: 0;
        left: 0;
        width: 100%;
        height: 100%;
        background-color: rgba(0,0,0,0.6);
        z-index: 100;
        visibility: hidden;
        opacity: 0;
        transition: 0.5s;
        overflow-y: scroll;
    }
    .mostrarModal{
        visibility: visible;
        opacity: 1;
    }
    @media(min-width: 1024px){
        .container{
            width: 80%;
        }
        .card{
            width: 50%;
            margin: 0 auto;
            margin-top: 30px;
        }
        .tabla{
            font-size: 14px;
        }
    }
    </style>
<body>
    <header>
        <div class="container">
            <h1>Productos</h1>
        </div>
    </header>
    <div class="container">
        <div class="row">
            <table class="tabla" cellspacing="0">
                <thead>
                    <th>idBoton</th>
                    <th>Descripcion</th>
                    <th>Precio</th>
                    <th>Cantidad</th>
                    <th>Tiempo(seg)</th>
                </thead>
                <tbody id="productos">    
                </tbody>
            </table>
        </div>
        <div class="row">
            <button type='submit' id='nuevoProducto'>Nuevo Producto</button>
        </div>
        <div class="row">
            <form id='configuracion' class="card">
                <div class="card-title">
                    <h2>Configuración</h2>    
                </div>
                <div class="card-body">
                    <p>Notificar cuando falten productos: </p>
                    <input type='number' id='notifiNum' required>
                    <p>Al correo: </p>
                    <input type='email' id='email' required>
                    <button type='submit' id='guardarconfig'>Guardar</button>
                </div>
            </form>
            <div id="observaciones"></div>
        </div>
    </div>
    <div id='modalNuevo' class='modal'>
        <div class="container">
            <form id='nuevoProductoModal' class="card card-space">
                <div class="card-title">
                    <h3>Productos</h3>
                </div>  
                <div class="card-body">
                    Descripcion <input type='text' id='descripcion' required>
                    Precio <input type='number' step="any" id='precio' required>
                    Cantidad <input type='number' id='cantidad' required>
                    Tiempo <input type='number' id='tiempo' required>
                    <button type='submit' id='guardarProducto'>Guardar Producto</button>
                    <button type='button' id='cancelarProducto'>Cancelar</button>
                </div>  
            </form>
        </div>
    </div>
    <div id='menuProductos' class='modal'>
        <div class="container">
            <div class='card card-space'>
                <div class="card-title">
                    <h3>Menu de acciones</h3>
                </div>
                <div class="card-body">
                    <button id='editarProducto'>Editar</button>
                    <button id='borrarProducto'>Borrar</button>
                    <button id='verPuertos'>Ver Puertos</button>
                    <button id='cancelarMenuPuerto'>Cancelar</button>
                </div>
            </div>
        </div>
    </div>
    <div id='modalBorrarProducto' class='modal'>
        <div class="container">
            <div class='card card-space'>
                <div class="card-title">
                    <h3>Estas seguro?...</h3>
                </div>
                <div class="card-body">
                    <button id='borrarProductoSI'>Borrar</button>
                    <button id='cancelarBorrar'>Cancelar</button>
                </div>
            </div>
        </div>
    </div>
    <div id='modalPuerto' class='modal'>
        <div class="container">
            <div class='card card-space'>
                <div class="card-title">
                    <h2>Puertos y cantidades</h2>
                </div>
                <div class="card-body"> 
                    <table class="tabla" cellspacing="0">
                        <thead>
                            <th>Puerto</th>
                            <th>Cantidad</th>
                        </thead>
                        <tbody id="puertos">
                        </tbody>
                    </table>
                    <button id='nuevoPuerto'>Agregar puerto</button>
                    <button id='cerrarModalPuerto'>Cerrar</button>
                </div>
            </div>
        </div>
    </div>
    <div id='modalNuevoPuerto' class='modal'>
        <div class="container">
            <form id='nuevoPuertoModal' class="card card-space">
                <div class="card-title">
                    <h3>Puertos</h3>
                </div>
                <div class="card-body"> 
                    Puerto <input type='number' step="any" min="0" max="260" id='puerto' required>
                    Cantidad <input type='number' id='cantidadPuerto' required>
                    <button type='submit' id='guardarPuerto'>Guardar Puerto</button>
                    <button type='button' id='cancelarNuevoPuerto'>Cancelar</button>
                </div>
            </form>
        </div>
    </div>
    <div id='menuPuertos' class='modal'>
        <div class="container">
            <div class='card card-space'>
                <div class="card-title">
                    <h3>Menu de acciones</h3>
                </div>
                <div class="card-body"> 
                    <button id='editarPuerto'>Editar</button>
                    <button id='borrarPuerto'>Borrar</button>
                    <button id='cancelarPuerto'>Cancelar</button>
                </div>
            </div>
        </div>
    </div>
    <div id='modalBorrarPuerto' class='modal'>
        <div class="container">
            <div class='card card-space'>
                <div class="card-title">
                    <h3>Estas seguro?...</h3>
                </div>
                <div class="card-body"> 
                    <button id='borrarPuertoSI'>Borrar</button>
                    <button id='cancelarBorrarPuerto'>Cancelar</button>
                </div>
            </div>
        </div>
    </div>
    <script>
        let accion = 'Productos';
        let accionProducto;
        let accionPuerto;
        let datos;
        let idseleccionado; 
        let idseleccionadoPuerto;
        let comando;
        let Descripcion = document.getElementById('descripcion');
        let Cantidad = document.getElementById('cantidad');
        let Precio = document.getElementById('precio');
        let tiempo = document.getElementById('tiempo');
        let puerto = document.getElementById('puerto');
        let cantidadPuerto = document.getElementById('cantidadPuerto');
        let email = document.getElementById('email');
        let notificacion = document.getElementById('notifiNum');
        let numconfig = 0;
        var obteneridProducto = (id)=>{
            idseleccionado = id;
            document.getElementById('menuProductos').classList.add('mostrarModal');
        }
        var obteneridPuerto = (id)=>{
            idseleccionadoPuerto = id;
            document.getElementById('menuPuertos').classList.add('mostrarModal');
        }
        let enviarValor = (comando, idBoton, Descripcion, Cantidad, Precio, tiempo)=>{
            let dato = `${comando},${idBoton},${descripcion.value},${cantidad.value},${precio.value},${tiempo.value},`;
            console.log('Enviando : ' + dato + ' al ESP8266');
            connection.send(dato);
            document.getElementById('cancelarProducto').disabled = true;
            document.getElementById('guardarProducto').disabled = true;
            descripcion.value = "";
            cantidad.value = "";
            precio.value = "";
            tiempo.value = "";
            setTimeout(()=>{
                document.getElementById('modalNuevo').classList.remove('mostrarModal');
                document.getElementById('cancelarProducto').disabled = false;
                document.getElementById('guardarProducto').disabled = false;
            }, 4000); 
        }
        let enviarValorPuerto = (comando, idBoton, puertos, cantidad)=>{
            let dato;
            if(comando == 4){
                dato = `${comando},${idBoton},,,,,${puertos.value},${cantidad.value},`;
            }else if(comando == 5){
                dato = `${comando},${idBoton},,,,,${puertos},${cantidad.value},`;
            }            
            console.log('Enviando : ' + dato + ' al ESP8266');
            connection.send(dato);
            document.getElementById('guardarPuerto').disabled = true;
            document.getElementById('cancelarNuevoPuerto').disabled = true;
            setTimeout(()=>{
                document.getElementById('modalNuevoPuerto').classList.remove('mostrarModal');
                document.getElementById('cancelarNuevoPuerto').disabled = false;
                document.getElementById('guardarPuerto').disabled = false;
            }, 4000); 
        }
        const connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);
        connection.onopen = ()=>{
            console.log("Conexion exitosa a WebSocket");
        }
        connection.onerror = (error)=>{
            console.log('Error en el WebSocket ', error);
        }
        connection.onmessage = (event)=>{
            var variable = event.data; 
            if(variable[0] == "["){
               var regex = new RegExp("'", "g");
                var datos = variable.replace(regex, "\""); 
                console.log('Resibiendo:' + datos + ".");
                let datosJSON = JSON.parse(datos);
                let registros = '';
                if(accion == 'Productos'){
                    for(i in datosJSON){
                        if(i == 0){
                            document.getElementById('observaciones').innerHTML = "<b>Se notificara a "+datosJSON[0].email+" cuando un producto llegue a la cantidad de "+datosJSON[0].notificacion+".</b>";
                        }else{
                            registros+= "<tr id="+datosJSON[i].idBoton+" onclick='obteneridProducto(this.id)'>";
                            registros+= '<td>'+datosJSON[i].idBoton+'</td>';
                            registros+= '<td>'+datosJSON[i].Descripcion+'</td>';
                            registros+= '<td>'+datosJSON[i].Precio+'</td>';
                            registros+= '<td>'+datosJSON[i].Cantidad+'</td>';
                            registros+= '<td>'+datosJSON[i].tiempo+'</td>';
                            registros+= '<tr>';
                        }
                    }
                    document.getElementById('productos').innerHTML = registros;
                }else if(accion == 'Puertos'){
                    for(k=0; k<datosJSON.puertos.length; k++){
                        registros+= "<tr id="+datosJSON.puertos[k]+" onclick='obteneridPuerto(this.id)'>";
                        registros+= '<td>'+datosJSON.puertos[k]+'</td>';
                        registros+= '<td>'+datosJSON.cantidad[k]+'</td>';
                        registros+= '<tr>';
                    }                    
                    document.getElementById('puertos').innerHTML = registros;
                }else if(accion == 'Configuracion'){
                    document.getElementById('observaciones').innerHTML = "<b>Se notificara a "+datosJSON.email+" cuando un producto llegue a la cantidad de "+datosJSON.notificacion+".</b>";
                    email.value = "";
                    notificacion.value = "";
                }
            }else{ 
                let arregloDeDato = variable.split(",");
            }
        }
        document.getElementById('nuevoProducto').addEventListener('click', ()=>{
            accion = 'Productos'
            accionProducto = 'Nuevo'
            document.getElementById('modalNuevo').classList.add('mostrarModal');
        });
        document.getElementById('editarProducto').addEventListener('click', ()=>{
            accion = 'Productos'
            accionProducto = 'Editar';
            document.getElementById('menuProductos').classList.remove('mostrarModal');
            document.getElementById('modalNuevo').classList.add('mostrarModal');
        });
        document.getElementById('borrarProducto').addEventListener('click', ()=>{
            document.getElementById('menuProductos').classList.remove('mostrarModal');
            document.getElementById('modalBorrarProducto').classList.add('mostrarModal');
        });        
        document.getElementById('cancelarBorrar').addEventListener('click', ()=>{
            document.getElementById('modalBorrarProducto').classList.remove('mostrarModal');
            document.getElementById('menuProductos').classList.add('mostrarModal');
        });
        document.getElementById('borrarProductoSI').addEventListener('click', ()=>{ 
            accion = 'Productos'
            let idBoton = idseleccionado;
            comando = 2;
            let dato = `${comando},${idBoton},`;
            console.log('Enviando : ' + dato + ' al ESP8266');
            connection.send(dato);
            document.getElementById('borrarProductoSI').disabled = true;
            document.getElementById('cancelarBorrar').disabled = true;
            setTimeout(()=>{
                document.getElementById('modalBorrarProducto').classList.remove('mostrarModal');
                document.getElementById('menuProductos').classList.remove('mostrarModal');
                document.getElementById('borrarProductoSI').disabled = false;
                document.getElementById('cancelarBorrar').disabled = false;
            }, 4000);
        });
        document.getElementById('cancelarProducto').addEventListener('click', ()=>{
            document.getElementById('modalNuevo').classList.remove('mostrarModal');
        });
        document.getElementById('nuevoProductoModal').addEventListener('submit', function(event){
            event.preventDefault();
            if(accionProducto == 'Nuevo'){
                comando = 0;
                let idBoton = generateUUID();
                enviarValor(comando, idBoton, Descripcion, Cantidad, Precio, tiempo);
            }else if(accionProducto == 'Editar'){
                let idBoton = idseleccionado;
                comando = 1;
                enviarValor(comando, idBoton, Descripcion, Cantidad, Precio, tiempo);
            }
        });
        document.getElementById('cancelarMenuPuerto').addEventListener('click', ()=>{
            document.getElementById('menuProductos').classList.remove('mostrarModal');
        });
        /*PUERTOS*/
        document.getElementById('verPuertos').addEventListener('click', ()=>{
            accion = 'Puertos';
            let idBoton = idseleccionado;
            comando = 3;
            let dato = `${comando},${idBoton},`;
            console.log('Enviando : ' + dato + ' al ESP8266');
            connection.send(dato);
            document.getElementById('editarProducto').disabled = true;
            document.getElementById('borrarProducto').disabled = true;
            document.getElementById('verPuertos').disabled = true;
            document.getElementById('cancelarMenuPuerto').disabled = true;
            setTimeout(()=>{
                document.getElementById('menuProductos').classList.remove('mostrarModal');
                document.getElementById('modalPuerto').classList.add('mostrarModal');
                document.getElementById('editarProducto').disabled = false;
                document.getElementById('borrarProducto').disabled = false;
                document.getElementById('verPuertos').disabled = false;
                document.getElementById('cancelarMenuPuerto').disabled = false;
            }, 2000);
        });
        document.getElementById('nuevoPuerto').addEventListener('click', ()=>{
            accion = 'Puertos';
            accionPuerto = 'Nuevo';
            puerto.value = "";
            cantidadPuerto.value = "";
            document.getElementById('modalNuevoPuerto').classList.add('mostrarModal');
        });
        document.getElementById('cerrarModalPuerto').addEventListener('click', ()=>{
            document.getElementById('menuProductos').classList.add('mostrarModal');
            document.getElementById('modalPuerto').classList.remove('mostrarModal');
        });
        document.getElementById('cancelarNuevoPuerto').addEventListener('click', ()=>{
            document.getElementById('modalNuevoPuerto').classList.remove('mostrarModal');
            document.getElementById('puerto').disabled = false;
        });      
        document.getElementById('nuevoPuertoModal').addEventListener('submit', function(event){
            event.preventDefault();
            if(accionPuerto == 'Nuevo'){
                comando = 4;
                let idBoton = idseleccionado;
                enviarValorPuerto(comando, idBoton, puerto, cantidadPuerto);
            }else if(accionPuerto == 'Editar'){
                comando = 5;
                let idBoton = idseleccionado;   
                enviarValorPuerto(comando, idBoton, idseleccionadoPuerto, cantidadPuerto);
            }
        });
        document.getElementById('cancelarPuerto').addEventListener('click', ()=>{
            document.getElementById('menuPuertos').classList.remove('mostrarModal');
        }); 
        document.getElementById('editarPuerto').addEventListener('click', ()=>{
            accion = 'Puertos';
            accionPuerto = 'Editar';
            puerto.value = "";
            cantidadPuerto.value = "";
            document.getElementById('puerto').disabled = true;
            document.getElementById('menuPuertos').classList.remove('mostrarModal');
            document.getElementById('modalNuevoPuerto').classList.add('mostrarModal');
        });
        document.getElementById('borrarPuerto').addEventListener('click', ()=>{
            document.getElementById('menuPuertos').classList.remove('mostrarModal');
            document.getElementById('modalBorrarPuerto').classList.add('mostrarModal');
        }); 
        document.getElementById('cancelarBorrarPuerto').addEventListener('click', ()=>{
            document.getElementById('modalBorrarPuerto').classList.remove('mostrarModal');
            document.getElementById('menuPuertos').classList.add('mostrarModal');
        });
        document.getElementById('borrarPuertoSI').addEventListener('click', ()=>{ 
            accion = 'Puertos'
            let idBoton = idseleccionado;
            comando = 6;
            let dato = `${comando},${idBoton},,,,,${idseleccionadoPuerto},,`
            console.log('Enviando : ' + dato + ' al ESP8266');
            connection.send(dato);
            document.getElementById('borrarPuertoSI').disabled = true;
            document.getElementById('cancelarBorrarPuerto').disabled = true;
            setTimeout(()=>{
                document.getElementById('modalBorrarPuerto').classList.remove('mostrarModal');
                document.getElementById('menuPuertos').classList.remove('mostrarModal');
                document.getElementById('borrarPuertoSI').disabled = false;
                document.getElementById('cancelarBorrarPuerto').disabled = false;
            }, 4000);
        });            
        document.getElementById('configuracion').addEventListener('submit', function(event){
            event.preventDefault();
            accion = 'Configuracion';
            comando = 7;
            let dato = `${comando},,,,,,,,${email.value},${notificacion.value},`;
            console.log('Enviando : ' + dato + ' al ESP8266');
            connection.send(dato);
            document.getElementById('guardarconfig').disabled = true;
            document.getElementById('nuevoProducto').disabled = true;
            setTimeout(()=>{
                document.getElementById('guardarconfig').disabled = false;
                document.getElementById('nuevoProducto').disabled = false;
            }, 4000);
        });
        function generateUUID(){
            var d = new Date().getTime();
            var uuid = 'xxxx4x'.replace(/[xy]/g, function (c) {
                var r = (d + Math.random() * 16) % 16 | 0;
                d = Math.floor(d / 16);
                return (c == 'x' ? r : (r & 0x3 | 0x8)).toString(16);
            });
            return uuid;
        }
    </script>
</body>
</html>
)";

ESP8266WebServer server(80);
WebSocketsServer webSocket(81);

String quitarComa(String datosEnviar){
    if(datosEnviar[1] == ','){
        datosEnviar.remove(1,1);
    }
    return datosEnviar;
}
String remplazarComillas(String datosSpiffs){
    String stringTwo = datosSpiffs;
    stringTwo.replace("'", "\"");
    return stringTwo;
}

void cargarConfiguracion(const char *filename, Config &config, String idBoton, String Descripcion, String Cantidad, String Precio, String tiempo){
    File file = SPIFFS.open(filename, "r");
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, file);
    if(error)
        Serial.println(F("Fallo al leer el archivo para editar"));
    config.idBoton = idBoton;
    config.Descripcion = Descripcion;
    config.Cantidad = Cantidad;
    config.Precio = Precio;
    config.tiempo = tiempo;
    file.close();
}

void guardarConfiguracion(const char *filename, const Config &config){
    File file = SPIFFS.open(filename, "r");
    if(!file){
        Serial.println(F("Fallo al crear archivo"));
        return;
    }
    String datosSpiffs;
    while(file.available()){
        datosSpiffs += (char)file.read();
    }
    String stringTwo = remplazarComillas(datosSpiffs);
    DynamicJsonDocument doc(1024);
    JsonArray arreglo = doc.to<JsonArray>();
    DeserializationError error = deserializeJson(doc, stringTwo);
    if(error)
        Serial.println(F("Fallo al leer el archivo para editar"));
    int longitud = arreglo.size();
    for(int i=0; i<longitud; i++){
        JsonObject objeto = arreglo[i];
        String idboton = objeto["idBoton"].as<String>();
        String confidboton = String(config.idBoton);
        if(idboton == confidboton){
            objeto["Descripcion"] = config.Descripcion;
            objeto["Cantidad"] = config.Cantidad;
            objeto["Precio"] = config.Precio;
            objeto["tiempo"] = config.tiempo;
        }
    }
    String datoSpiffs = doc.as<String>();
    file.close();
    SPIFFS.format();
    File file2 = SPIFFS.open(filename, "a+");
    if(!file2){
        Serial.println(F("Fallo al crear archivo"));
        return;
    }
    file2.print(datoSpiffs);
    file2.close();
}
void borrarDato(const char *filename, String idBoton){
    File file = SPIFFS.open(filename, "r");
    if(!file){
        Serial.println(F("Fallo al crear archivo"));
        return;
    }
    DynamicJsonDocument doc(1024);
    JsonArray arreglo = doc.to<JsonArray>();
    DeserializationError error = deserializeJson(doc, file);
    if(error)
        Serial.println(F("Fallo al leer el archivo para editar"));
    int longitud = arreglo.size();
    int indice = 0;
    for(int i=0; i<longitud; i++){
        JsonObject objeto = arreglo[i];
        String idboton = objeto["idBoton"].as<String>();
        if(idboton == idBoton){
            indice = i;
        }
    }
    arreglo.remove(indice);
    String datoSpiffs = doc.as<String>();
    file.close();
    SPIFFS.format();
    File file2 = SPIFFS.open(filename, "a+");
    if(!file2){
        Serial.println(F("Fallo al crear archivo"));
        return;
    }
    file2.print(datoSpiffs);
    file2.close();
}

String mostrarDatos(const char *filename){
    File file = SPIFFS.open(filename, "r");
    String datosSpiffs;
    if(!file){
        Serial.println(F("Fallo al leer el archivo"));
    }
    while(file.available()){
        datosSpiffs += (char)file.read();
    }
    String stringTwo = remplazarComillas(datosSpiffs);
    Serial.println(stringTwo);
    file.close();
    return datosSpiffs;
}

String parcearJSON(String idBoton, String Descripcion, String Cantidad, String Precio,String tiempo){
    String datosNuevos = ",{'idBoton':'"+idBoton+"','Descripcion':'"+Descripcion+"','Cantidad':"+Cantidad.toInt()+",'Precio':"+Precio.toInt()+",'tiempo':"+tiempo.toInt()+",'puertos':[],'cantidad':[]}]";
    return datosNuevos;
}

void agregarDato(const char *filename, String datosNuevos){
    File file = SPIFFS.open(filename, "r");
    String datosSpiffs;
    if(!file){
        Serial.println(F("Fallo al leer el archivo"));
    }
    while(file.available()){
        datosSpiffs += (char)file.read();
    }
    int eliminar = datosSpiffs.length()-1;
    datosSpiffs.remove(eliminar);
    datosSpiffs = datosSpiffs + datosNuevos;
    file.close();
    SPIFFS.format();
    File file2 = SPIFFS.open(filename, "a+");
    if(!file2){
        Serial.println(F("Fallo al crear archivo"));
        return;
    }
    file2.print(datosSpiffs);
    file2.close();
}

String obtenerDatosPuerto(const char *filename, String idBoton){
    File file = SPIFFS.open(filename, "r");
    if(!file){
        Serial.println(F("Fallo al crear archivo"));
    }
    String datosSpiffs;
    while(file.available()){
        datosSpiffs += (char)file.read();
    }
    String stringTwo = remplazarComillas(datosSpiffs);
    DynamicJsonDocument doc(1024);
    JsonArray arreglo = doc.to<JsonArray>();
    DeserializationError error = deserializeJson(doc, stringTwo);
    if(error)
        Serial.println(F("Fallo al leer el archivo para editar"));
    int longitud = arreglo.size();
    String puertos;
    String cantidad;
    String Sub;
    for(int i=0; i<longitud; i++){
        JsonObject objeto = arreglo[i];
        String idboton = objeto["idBoton"].as<String>();
        if(idboton == idBoton){
            puertos = objeto["puertos"].as<String>();
            cantidad = objeto["cantidad"].as<String>();
            Sub = "{'puertos':"+puertos+",'cantidad':"+cantidad+"}";
        }
    }
    file.close();
    return Sub;    
}
void nuevoPuerto(const char *filename, String idBoton, String puertos, String cantidad){
    File file = SPIFFS.open(filename, "r");
    if(!file){
        Serial.println(F("Fallo al crear archivo"));
    }
    String datosSpiffs;
    while(file.available()){
        datosSpiffs += (char)file.read();
    }
    String stringTwo = remplazarComillas(datosSpiffs);
    DynamicJsonDocument doc(1024);
    JsonArray arreglo = doc.to<JsonArray>();
    DeserializationError error = deserializeJson(doc, stringTwo);
    if(error)
        Serial.println(F("Fallo al leer el archivo para editar"));
    int longitud = arreglo.size();
    for(int i=0; i<longitud; i++){
        JsonObject objeto = arreglo[i];
        String idboton = objeto["idBoton"].as<String>();
        if(idboton == idBoton){
            JsonArray puertosArray = objeto["puertos"];
            JsonArray cantidadArray = objeto["cantidad"];
            puertosArray.add(puertos);
            cantidadArray.add(cantidad);
        }
    }
    String datoSpiffs = doc.as<String>();
    Serial.println(datoSpiffs);
    file.close();
    SPIFFS.format();
    File file2 = SPIFFS.open(filename, "a+");
    if(!file2){
        Serial.println(F("Fallo al crear archivo"));
        return;
    }
    file2.print(datoSpiffs);
    file2.close();
}
void editarPuerto(const char *filename, String idBoton, String puertos, String cantidad){
    File file = SPIFFS.open(filename, "r");
    if(!file){
        Serial.println(F("Fallo al crear archivo"));
    }
    String datosSpiffs;
    while(file.available()){
        datosSpiffs += (char)file.read();
    }
    String stringTwo = remplazarComillas(datosSpiffs);
    DynamicJsonDocument doc(1024);
    JsonArray arreglo = doc.to<JsonArray>();
    DeserializationError error = deserializeJson(doc, stringTwo);
    if(error)
        Serial.println(F("Fallo al leer el archivo para editar"));
    int longitud = arreglo.size();
    for(int i=0; i<longitud; i++){
        JsonObject objeto = arreglo[i];
        String idboton = objeto["idBoton"].as<String>();
        if(idboton == idBoton){
            JsonArray puertosArray = objeto["puertos"];
            JsonArray cantidadArray = objeto["cantidad"];
            int longitudArray = puertosArray.size();
            for(int k=0; k<longitudArray; k++){
                String puertoArra = puertosArray[k].as<String>();
                if(puertoArra == puertos){
                    cantidadArray[k] = cantidad;
                }
            }
            
        }
    }
    String datoSpiffs = doc.as<String>();
    Serial.println(datoSpiffs);
    file.close();
    SPIFFS.format();
    File file2 = SPIFFS.open(filename, "a+");
    if(!file2){
        Serial.println(F("Fallo al crear archivo"));
        return;
    }
    file2.print(datoSpiffs);
    file2.close();
}
void borrarPuerto(const char *filename, String idBoton, String puertos){
    File file = SPIFFS.open(filename, "r");
    if(!file){
        Serial.println(F("Fallo al crear archivo"));
    }
    String datosSpiffs;
    while(file.available()){
        datosSpiffs += (char)file.read();
    }
    String stringTwo = remplazarComillas(datosSpiffs);
    DynamicJsonDocument doc(1024);
    JsonArray arreglo = doc.to<JsonArray>();
    DeserializationError error = deserializeJson(doc, stringTwo);
    if(error)
        Serial.println(F("Fallo al leer el archivo para editar"));
    int longitud = arreglo.size();
    for(int i=0; i<longitud; i++){
        JsonObject objeto = arreglo[i];
        String idboton = objeto["idBoton"].as<String>();
        if(idboton == idBoton){
            JsonArray puertosArray = objeto["puertos"];
            JsonArray cantidadArray = objeto["cantidad"];
            int longitudArray = puertosArray.size();
            for(int k=0; k<longitudArray; k++){
                String puertoArra = puertosArray[k].as<String>();
                if(puertoArra == puertos){
                    puertosArray.remove(k);
                    cantidadArray.remove(k);
                }
            }
            
        }
    }
    String datoSpiffs = doc.as<String>();
    Serial.println(datoSpiffs);
    file.close();
    SPIFFS.format();
    File file2 = SPIFFS.open(filename, "a+");
    if(!file2){
        Serial.println(F("Fallo al crear archivo"));
        return;
    }
    file2.print(datoSpiffs);
    file2.close();
}
String obtenerConfiguracion(const char *filename){
    File file = SPIFFS.open(filename, "r");
    if(!file){
        Serial.println(F("Fallo al crear archivo"));
    }
    String datosSpiffs;
    while(file.available()){
        datosSpiffs += (char)file.read();
    }
    String stringTwo = remplazarComillas(datosSpiffs);
    DynamicJsonDocument doc(1024);
    JsonArray arreglo = doc.to<JsonArray>();
    DeserializationError error = deserializeJson(doc, stringTwo);
    if(error)
        Serial.println(F("Fallo al leer el archivo para editar"));
    String configuracion = arreglo[0].as<String>();
    file.close();
    return configuracion;
}
void guardarConfiguracion(const char *filename, String email, String notificacion){
    File file = SPIFFS.open(filename, "r");
    if(!file){
        Serial.println(F("Fallo al crear archivo"));
    }
    String datosSpiffs;
    while(file.available()){
        datosSpiffs += (char)file.read();
    }
    String stringTwo = remplazarComillas(datosSpiffs);
    DynamicJsonDocument doc(1024);
    JsonArray arreglo = doc.to<JsonArray>();
    DeserializationError error = deserializeJson(doc, stringTwo);
    if(error)
        Serial.println(F("Fallo al leer el archivo para editar"));
    JsonObject configuracion = arreglo[0];
    configuracion["email"] = email;
    configuracion["notificacion"] = notificacion;
    file.close();
    
    String datoSpiffs = doc.as<String>();
    Serial.println(datoSpiffs);
    file.close();
    SPIFFS.format();
    File file2 = SPIFFS.open(filename, "a+");
    if(!file2){
        Serial.println(F("Fallo al crear archivo"));
        return;
    }
    file2.print(datoSpiffs);
    file2.close();
}
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length){
    switch(type){
        case WStype_CONNECTED: {
            IPAddress ip = webSocket.remoteIP(num);
            Serial.print("Cliente ");
            Serial.print(ip);
            Serial.println(" conectado!");
            //Serial.println(num);
            String datosEnviar = mostrarDatos(filename);
            datosEnviar = quitarComa(datosEnviar);
            webSocket.sendTXT(num, datosEnviar);
        }
        break;
        case WStype_DISCONNECTED: {
            Serial.println("Desconectado!");
            File file = SPIFFS.open(filename, "r");
            if(!file){
                Serial.println(F("Fallo al crear archivo"));
            }
            String datosSpiffs;
            while(file.available()){
                datosSpiffs += (char)file.read();
            }
            file.close();
            String stringTwo = remplazarComillas(datosSpiffs);
            if(stringTwo[1] == ','){
                stringTwo.remove(1,1);
                SPIFFS.format();
                File file2 = SPIFFS.open(filename, "a+");
                if(!file2){
                    Serial.println(F("Fallo al crear archivo"));
                }
                file2.print(stringTwo);
                file2.close();
            }
        }
        break;
        case WStype_TEXT:
            String comando;
            String idBoton;
            String Descripcion;
            String Cantidad;
            String Precio;
            String tiempo;
            String puertos;
            String cantidad;
            String email;
            String notificacion;
            int c = 0;
            for(int i=0; i<length; i++) {
                if((char)payload[i] == ','){
                    c += 1; 
                }else{
                    if(c == 0){
                        comando = comando + (char)payload[i];
                    }
                    if(c == 1){
                        idBoton = idBoton + (char)payload[i];
                    }
                    if(c == 2){
                        Descripcion = Descripcion + (char)payload[i];
                    }
                    if(c == 3){
                        Cantidad = Cantidad + (char)payload[i];
                    }
                    if(c == 4){
                        Precio = Precio + (char)payload[i];
                    }
                    if(c == 5){
                        tiempo = tiempo + (char)payload[i];
                    }
                    if(c == 6){
                        puertos = puertos + (char)payload[i];
                    }
                    if(c == 7){
                        cantidad = cantidad + (char)payload[i];
                    }
                    if(c == 8){
                        email = email + (char)payload[i];
                    }
                    if(c == 9){
                        notificacion = notificacion + (char)payload[i];
                    }
                }
            }
            Serial.println("comado: "+comando);
            Serial.println("idBoton: "+idBoton);
            Serial.println("Descripcion: "+Descripcion);
            Serial.println("Cantidad: "+Cantidad);
            Serial.println("Precio: "+Precio);
            Serial.println("tiempo: "+tiempo);
            Serial.println("puertos: "+puertos);
            Serial.println("cantidad: "+cantidad);
            if(comando == "0"){
                String datoParceado = parcearJSON(idBoton, Descripcion, Cantidad, Precio, tiempo);
                agregarDato(filename, datoParceado);
                String datosEnviar = mostrarDatos(filename);
                Serial.println(datosEnviar[1]);
                if(datosEnviar[1] == ','){
                    datosEnviar.remove(1,1);
                    Serial.println(datosEnviar);
                }
                webSocket.sendTXT(num, datosEnviar);
            }else if(comando == "1"){
                cargarConfiguracion(filename, config, idBoton, Descripcion, Cantidad, Precio, tiempo);
                guardarConfiguracion(filename, config);
                String datosEnviar = mostrarDatos(filename);
                webSocket.sendTXT(num, datosEnviar);
            }else if(comando == "2"){
                borrarDato(filename, idBoton);
                String datosEnviar = mostrarDatos(filename);
                webSocket.sendTXT(num, datosEnviar);
            }else if(comando == "3"){
                String Sub = obtenerDatosPuerto(filename, idBoton);
                Serial.println("Enviando... "+Sub);
                webSocket.sendTXT(num, Sub);
            }else if(comando == "4"){
                nuevoPuerto(filename, idBoton, puertos, cantidad);
                String Sub = obtenerDatosPuerto(filename, idBoton);
                Serial.println("Enviando... "+Sub);
                webSocket.sendTXT(num, Sub);
            }else if(comando == "5"){
                editarPuerto(filename, idBoton, puertos, cantidad);
                String Sub = obtenerDatosPuerto(filename, idBoton);
                Serial.println("Enviando... "+Sub);
                webSocket.sendTXT(num, Sub);
            }else if(comando == "6"){
                borrarPuerto(filename, idBoton, puertos);
                String Sub = obtenerDatosPuerto(filename, idBoton);
                Serial.println("Enviando... "+Sub);
                webSocket.sendTXT(num, Sub);
            }else if(comando == "7"){
                guardarConfiguracion(filename, email, notificacion);
                String datosEnviar = obtenerConfiguracion(filename);
                webSocket.sendTXT(num, datosEnviar);
            }
        break;
    }
}
void paginaPrincipal(){
    server.send(200, "text/html", INDEX_HTML);
}

void setup(){
    pinMode(16, OUTPUT);
    Serial.begin(115200);
    WiFi.softAP(ssid, password);
    IPAddress myIP = WiFi.softAPIP();
    WiFi.softAPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));
    Serial.print("Direccion IP del acceso: ");
    Serial.println(myIP);
    if(!SPIFFS.begin()){
        Serial.println("Fallo montando el documento");
        return;
    }
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    server.on("/", paginaPrincipal);
    
    server.begin();
    Serial.println("WebServer iniciado...");
}
void loop(){
    webSocket.loop();
    server.handleClient();
    if(digitalRead(16) == 0) {
        strPulsador = "presionado";
    }else{
        strPulsador = "NO presionado";
    }
    if(strPulsador != strPulsadorUltimo){
        strPulsadorUltimo = strPulsador;
        String mandarBoton = "btn,"+strPulsador+",16,";
        int str_len = mandarBoton.length() + 1; 
        char char_array[str_len];
        mandarBoton.toCharArray(char_array, str_len);
        webSocket.broadcastTXT(char_array, str_len);
    }
}
