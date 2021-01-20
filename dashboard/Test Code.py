import dash                                            # Dash library
import pandas as pd                                    # Pandas for data handling
import dash_core_components as dcc                     # Dash components library
import dash_html_components as html                    # HTML components library
import plotly.graph_objs as go                         # Plotly library
import random
import gspread
from dash.dependencies import Input, Output            # For callbacks
from collections import deque
#from oauth2client.service_account import ServiceAccountCredentials


# ------------------------------------------------ Data Handling --------------------------------------------------

# Read the data from .csv file
df = pd.read_csv('pH_Temp_data.csv')

# use creds to create a client to interact with the Google Drive API
scope = ['https://spreadsheets.google.com/feeds']
#creds = ServiceAccountCredentials.from_json_keyfile_name('client_secret.json', scope)
#client = gspread.authorize(creds)

# Find a workbook by name and open the first sheet
# Make sure you use the right name here.
#sheet = client.open("Sensor_Readings").sheet1

# Extract and print all of the values
#list_of_hashes = sheet.get_all_records()


X = deque(maxlen=150)
X.append(1)
Y = deque(maxlen=30)
Y.append(1)


# -------------------------------------------------   App Layout   --------------------------------------------------

app = dash.Dash(__name__)
app.layout = html.Div(

    html.Div(className='row',  # Define the row element
             children=[html.Div(className='four columns div-user-controls',  # Define the left side
                                children=[html.H1(html.Img(src=app.get_asset_url('logognd.png'), height=170))],
                                ),
                       html.Div(className='eight columns div-for-charts bg-grey',  # Define the right side
                                children=[dcc.Graph(id='live-graph', animate=True, config={"displayModeBar": False}),
                                          dcc.Interval(id='graph-update', interval=2000, n_intervals=8),
                                          dcc.Graph(id='ph-graph', animate=True, config={"displayModeBar": False}),
                                          dcc.Interval(id='ph-update', interval=2000, n_intervals=8)
                                          ]
                                )
                       ]
             )
)

# --------------------------------------   Temperature and pH graphs   -----------------------------------------------

@app.callback(Output('live-graph', 'figure'),
              Output('ph-graph', 'figure'),
              Input('graph-update', 'n_intervals'))

def update_graph_scatter(n):
    X.append(X[-1] + 1)
    Y.append(Y[-1] + Y[-1] * random.uniform(-0.1, 0.1))

    data = go.Scatter(x=list(X), y=df['Temperature (C)'],
                      name='Scatter',
                      mode='lines+markers', marker_size=5,
    )

    data = {'data': [data], 'layout': go.Layout(xaxis=dict(range=[min(X)-0.2, max(X)+0.5],
                                                           zeroline=False, showgrid=False,
                                                           title='Samples ( t = 2s )'),
                                                yaxis=dict(range=[12, 25],
                                                           zeroline=False, showgrid=False,
                                                           title='Temperature ( °C )'),
                                                title='Temperature Graph',
                                                plot_bgcolor = 'rgba(0, 0, 0, 0)',
                                                paper_bgcolor = 'rgba(0, 0, 0, 0)',
                                                font=dict(size=12, family='Arial', color='white'),
                                               )
           }

    ph_data = go.Scatter(x=list(X), y=df['pH'],
                         name='Scatter',
                         mode='lines+markers', marker_size=5,
                         )

    ph_data = {'data': [ph_data], 'layout': go.Layout(xaxis=dict(range=[min(X)-0.2, max(X)+0.5],
                                                           zeroline=False, showgrid=False,
                                                           title='Samples ( t = 2s )'),
                                                yaxis=dict(range=[2, 8],
                                                           zeroline=False, showgrid=False,
                                                           title='pH  [ 0 - 14 ]'),
                                                title='pH Graph',
                                                plot_bgcolor = 'rgba(0, 0, 0, 0)',
                                                paper_bgcolor = 'rgba(0, 0, 0, 0)',
                                                font=dict(size=12, family='Arial', color='white')
                                               )
              }

    return data, ph_data

# -------------------------------------------- Deploy app on local server ----------------------------------------


if __name__ == '__main__':
    app.run_server(debug = True)