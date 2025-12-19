import pymysql
import streamlit as st
import pandas as pd
import matplotlib.pyplot as plt

def fetch_data(host, user, password, database):
    try:
        # Conexión a la base de datos
        connection = pymysql.connect(
            host=host,
            user=user,
            password=password,
            database=database,
            port=12903
        )
        cursor = connection.cursor()
        cursor.execute(f"SELECT * FROM medicinav1 where nombre_sensor in ('DHT22')")
        data = cursor.fetchall()
        columns = [desc[0] for desc in cursor.description]  # Obtener los nombres de las columnas
        connection.close()
        return pd.DataFrame(data, columns=columns)
    except pymysql.MySQLError as e:
        st.error(f"Error al conectar a la base de datos: {e}")
        return pd.DataFrame()

def fetch_data_mov(host, user, password, database):
    try:
        # Conexión a la base de datos
        connection = pymysql.connect(
            host=host,
            user=user,
            password=password,
            database=database,
            port=12903
        )
        cursor = connection.cursor()
        cursor.execute(f"SELECT * FROM medicinav1 where nombre_sensor in ('MPU6050')")
        data = cursor.fetchall()
        columns = [desc[0] for desc in cursor.description]  # Obtener los nombres de las columnas
        connection.close()
        return pd.DataFrame(data, columns=columns)
    except pymysql.MySQLError as e:
        st.error(f"Error al conectar a la base de datos: {e}")
        return pd.DataFrame()

def main():
    #st.title('Bienvenido a PokMed')
    
    mensajeBienvenida = st.empty()
    mensajeB2 = st.empty()
    nombreIntro = st.empty()
    contraIntro = st.empty()
    
    #mensajeB2.subheader('Vamos a verificar como se encuentra tu insulina.')
    #mensajeBienvenida.write("Pero primero, por favor dinos quien eres:")

    n = nombreIntro.text_input("Usuario:")
    contra = contraIntro.text_input("Contraseña:")

    if True:
        #mensajeBienvenida.empty()
        mensajeB2.empty()
        nombreIntro.empty()
        contraIntro.empty()
        #st.subheader(f'Bienvenido {n}')

        host = "autorack.proxy.rlwy.net"
        user = "root"
        password = "QYruqXDRGGyBxlYXXcoMmaTSExlNQYxZ"
        database = "railway"

        # Cargar datos de temperatura
        df = fetch_data(host, user, password, database)
        if 'timestamp' in df.columns:
            df['timestamp'] = pd.to_datetime(df['timestamp'])
            df = df.sort_values(by='timestamp', ascending=True)
        # Cargar datos de movimiento
        df2 = fetch_data_mov(host, user, password, database)
        if 'timestamp' in df2.columns:
            df2['timestamp'] = pd.to_datetime(df2['timestamp'])
            df2 = df2.sort_values(by='timestamp', ascending=True)
    
        if df['valor'].iloc[-1] > 26:
            st.subheader("Tu insulina esta en malas condiciones en este momento! La temperatura supera los 26°")
        elif df['valor'].iloc[-1] < 2:
            st.subheader("Tu insulina esta en malas condiciones en este momento! La temperatura esta debajo de 2°")
        else:
            st.subheader("Todo parece bien con tu insulina!")
    col1, col2=st.columns(2)
    with col1:
        df = fetch_data(host, user, password, database)
        if 'timestamp' in df.columns:
            df['timestamp'] = pd.to_datetime(df['timestamp'])
            df = df.sort_values(by='timestamp', ascending=True)
        # Cargar datos de movimiento
        df2 = fetch_data_mov(host, user, password, database)
        if 'timestamp' in df2.columns:
            df2['timestamp'] = pd.to_datetime(df2['timestamp'])
            df2 = df2.sort_values(by='timestamp', ascending=True)
            
    
        # Crear la gráfica con puntos y colores personalizados
        fig, ax = plt.subplots()
    
        # Colores según las condiciones
        for i in range(len(df) - 1):  # Iterar sobre los puntos
            # Determinar el color del punto
            if df['valor'].iloc[i] > 26:  # Ajustar a tus valores
                color = '#FF7F3E'
            elif df['valor'].iloc[i] < 2:
                color = '#4335A7'
            else:
                color = '#80C4E9'
    
            # Graficar el punto
            ax.scatter(df['timestamp'].iloc[i], df['valor'].iloc[i], color=color, zorder=3)
    
            # Agregar el valor debajo del punto
            ax.text(
                df['timestamp'].iloc[i],
                df['valor'].iloc[i] - 0.25,  # Ajustar el valor para que el texto quede debajo
                f"{df['valor'].iloc[i]:.1f}",  # Formato con dos decimales
                color='white',
                fontsize=8,
                ha='center'  # Centrar el texto horizontalmente
            )
    
            # Determinar el color de la línea según el punto al que conecta
            if df['valor'].iloc[i + 1] > 26:
                line_color = '#FF7F3E'
            elif df['valor'].iloc[i + 1] < 2:
                line_color = '#4335A7'
            else:
                line_color = '#80C4E9'
    
            # Graficar la línea hasta el siguiente punto
            ax.plot(
                [df['timestamp'].iloc[i], df['timestamp'].iloc[i + 1]],
                [df['valor'].iloc[i], df['valor'].iloc[i + 1]],
                color=line_color,
                zorder=2
            )
    
        # Graficar el último punto
        if df['valor'].iloc[-1] > 29:
            last_color = '#FF7F3E'
        elif df['valor'].iloc[-1] < 26:
            last_color = '#4335A7'
        else:
            last_color = '#80C4E9'
    
        ax.scatter(df['timestamp'].iloc[-1], df['valor'].iloc[-1], color=last_color, zorder=3)
    
        # Agregar el valor al último punto
        ax.text(
            df['timestamp'].iloc[-1],
            df['valor'].iloc[-1] - 0.25,  # Ajustar el valor para que el texto quede debajo
            f"{df['valor'].iloc[-1]:.1f}",  # Formato con dos decimales
            color='white',
            fontsize=8,
            ha='center'
        )
        ax.set_ylim(df['valor'].min() - 1, df['valor'].max() + 1)
        # Lo de abajo en teoria amplia el alcance de la grafica en x, pero hasta ahora no ha hecho falta.
        # ax.set_xlim(df['timestamp'].min() - pd.Timedelta(seconds=5), df['timestamp'].max() + pd.Timedelta(seconds=5))  # Espacio horizontal
        # Ajustes del gráfico
        ax.set_title('Temperatura de la insulina', color='black')  # Título en blanco para destacar
        ax.set_xlabel('Fecha y hora de medición', color='black')  # Etiqueta del eje x en blanco
        ax.set_ylabel('Temperatura °C', color='black')  # Etiqueta del eje y en blanco
        plt.xticks(rotation=45, color='black')  # Rotar etiquetas del eje x y ponerlas en blanco
        plt.yticks(color='black')  # Etiquetas del eje y en blanco
    
        # Invertir el eje x para que las más recientes estén a la izquierda
        ax.invert_xaxis()
    
        # Mostrar la gráfica en Streamlit
        st.pyplot(fig)
        st.subheader("Tabla de Temperaturas")
        st.dataframe(df)
    with col2:
        fig2, ax2 = plt.subplots()

    # Colores según las condiciones
        for i in range(len(df2) - 1):  # Iterar sobre los puntos
            # Determinar el color del punto
            if df2['valor'].iloc[i] > 26:  # Ajustar a tus valores
                color = '#FF7F3E'
            elif df2['valor'].iloc[i] < 2:
                color = '#4335A7'
            else:
                color = '#80C4E9'
    
            # Graficar el punto
            ax2.scatter(df2['timestamp'].iloc[i], df2['valor'].iloc[i], color=color, zorder=3)
    
            # Agregar el valor debajo del punto
            ax2.text(
                df2['timestamp'].iloc[i],
                df2['valor'].iloc[i] - 0.25,  # Ajustar el valor para que el texto quede debajo
                f"{df2['valor'].iloc[i]:.1f}",  # Formato con dos decimales
                color='white',
                fontsize=8,
                ha='center'  # Centrar el texto horizontalmente
            )
    
            # Determinar el color de la línea según el punto al que conecta
            if df2['valor'].iloc[i + 1] > 2:
                line_color = '#FF7F3E'
            else:
                line_color = '#80C4E9'
    
            # Graficar la línea hasta el siguiente punto
            ax2.plot(
                [df2['timestamp'].iloc[i], df2['timestamp'].iloc[i + 1]],
                [df2['valor'].iloc[i], df2['valor'].iloc[i + 1]],
                color=line_color,
                zorder=2
            )
    
        # Graficar el último punto
        if df2['valor'].iloc[-1] > 2:
            last_color = '#FF7F3E'
        else:
            last_color = '#80C4E9'
    
        ax2.scatter(df2['timestamp'].iloc[-1], df2['valor'].iloc[-1], color=last_color, zorder=3)
    
        # Agregar el valor al último punto
        ax2.text(
            df2['timestamp'].iloc[-1],
            df2['valor'].iloc[-1] - 0.25,  # Ajustar el valor para que el texto quede debajo
            f"{df2['valor'].iloc[-1]:.1f}",  # Formato con dos decimales
            color='white',
            fontsize=8,
            ha='center'
        )
        ax2.set_ylim(df2['valor'].min() - 1, df2['valor'].max() + 1)
        # Lo de abajo en teoria amplia el alcance de la grafica en x, pero hasta ahora no ha hecho falta.
        # ax.set_xlim(df['timestamp'].min() - pd.Timedelta(seconds=5), df['timestamp'].max() + pd.Timedelta(seconds=5))  # Espacio horizontal
        # Ajustes del gráfico
        ax2.set_title('Movimiento de la insulina', color='black')  # Título en blanco para destacar
        ax2.set_xlabel('Fecha y hora de medición', color='black')  # Etiqueta del eje x en blanco
        ax2.set_ylabel('Movimiento', color='black')  # Etiqueta del eje y en blanco
        plt.xticks(rotation=45, color='black')  # Rotar etiquetas del eje x y ponerlas en blanco
        plt.yticks(color='black')  # Etiquetas del eje y en blanco
    
        # Invertir el eje x para que las más recientes estén a la izquierda
        ax2.invert_xaxis()
    
        # Mostrar la gráfica en Streamlit
        st.pyplot(fig2)
        st.subheader("Tabla de Movimiento")
        st.dataframe(df2)
    
if __name__ == "__main__":
    main()
